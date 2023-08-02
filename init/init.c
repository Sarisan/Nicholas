#include <api/api.h>
#include <config/config.h>
#include <init/init.h>
#include <debug/debug.h>
#include <legacy/legacy.h>
#include <parsers/parsers.h>
#include <threads.h>
#include <sankaku/api.h>
#include <stdatomic.h>

#define THREADS 64
#define POOL_SIZE 1024
#define SANKAKU_TIMER 36000
#define MAX_ERORRS 5

static atomic_bool init = true;
static json_object *global_config = 0;
static time_t sankaku_time[2] = {0};
static mtx_t sankaku_mtx;
static mtx_t pool_mtx;
static cnd_t sankaku_cnd;
static cnd_t pool_cnd;
static cnd_t pool_flow;
static size_t pool_count = 0;
static json_object *pool_list[POOL_SIZE] = {0};

static bool init_running(void)
{
    return atomic_load(&init);
}

static void init_stop(void)
{
    atomic_store(&init, false);
}

void init_signal(int signal)
{
    (void)signal;

    debug_log(0, "init_signal: Stopping the bot...");
    init_stop();
}

static int sankaku_thread()
{
    mtx_lock(&sankaku_mtx);

    while (init_running()) {
        cnd_wait(&sankaku_cnd, &sankaku_mtx);

        if (init_running())
            if (!sankaku_authorization(global_config))
                sankaku_time[0] = time(0);
    }

    mtx_unlock(&sankaku_mtx);

    return 0;
}

static void bot_parser(json_object *update)
{
    const char *command = command_message(update);
    const char *inlineq = inline_query(update);
    const char *callback = callback_data(update);

    if (command)
        bot_legacy_command(command, global_config, update);

    if (inlineq)
        bot_legacy_inline(inlineq, global_config, update);

    if (callback)
        bot_legacy_callback(callback, global_config, update);

    json_put(update);
}

static int init_pool()
{
    mtx_lock(&pool_mtx);

    while (init_running()) {
        json_object *update = 0;

        if (!pool_count)
            cnd_wait(&pool_cnd, &pool_mtx);

        if (!pool_count)
            continue;

        update = pool_list[0];

        for (size_t count = 0; count < pool_count - 1; count++)
            pool_list[count] = pool_list[count + 1];

        pool_count--;

        if (pool_count < POOL_SIZE)
            cnd_signal(&pool_flow);

        mtx_unlock(&pool_mtx);
        bot_parser(update);
        mtx_lock(&sankaku_mtx);

        sankaku_time[1] = time(0);

        if (difftime(sankaku_time[1], sankaku_time[0]) >= SANKAKU_TIMER)
            cnd_signal(&sankaku_cnd);

        mtx_unlock(&sankaku_mtx);
        mtx_lock(&pool_mtx);
    }

    mtx_unlock(&pool_mtx);

    return 0;
}

static void init_pool_add(json_object *update)
{
    mtx_lock(&pool_mtx);

    if (pool_count == POOL_SIZE)
        cnd_wait(&pool_flow, &pool_mtx);

    pool_list[pool_count] = update;
    pool_count++;

    cnd_signal(&pool_cnd);
    mtx_unlock(&pool_mtx);
}

int init_bot(json_object *config)
{
    int username = 0;
    int sankaku = 0;
    int64_t offset = 0;
    thrd_t thrd_sankaku;
    thrd_t thrd_bot[THREADS];
    uint8_t error_count = 0;

    global_config = config;
    username = api_username(config);

    if (username)
        return username;

    sankaku = sankaku_authorization(config);

    if (sankaku)
        return sankaku;

    sankaku_time[0] = time(0);
    offset = config_get_int64(config, BOT_OFFSET);

    if (!offset)
        offset = -1;

    mtx_init(&sankaku_mtx, 0);
    mtx_init(&pool_mtx, 0);
    cnd_init(&sankaku_cnd);
    cnd_init(&pool_cnd);
    cnd_init(&pool_flow);

    debug_log(0, "init_bot: Initializing threads...");

    if(thrd_create(&thrd_sankaku, sankaku_thread, 0))
        debug_log(EINV, "init_bot: Failed to initialize sankaku_thread thread");

    for (size_t threads = 0; threads < THREADS; threads++)
        if(thrd_create(&thrd_bot[threads], init_pool, 0))
            debug_log(EINV, "init_bot: Failed to initialize thread %ld", threads);

    debug_log(0, "init_bot: Done");

    while (init_running()) {
        json_object *update = api_update(config, offset);
        int64_t update_id = json_int64(update, "update_id");

        if(update_id) {
            offset = update_id + 1;

            init_pool_add(update);
        } else if (update) {
            error_count++;

            json_put(update);
        }

        if (error_count > MAX_ERORRS) {
            debug_log(0, "init_bot: More than 5 errors occurred, stopping...");
            init_stop();
        }
    }

    debug_log(0, "init_bot: Finishing threads...");

    mtx_lock(&sankaku_mtx);
    cnd_broadcast(&sankaku_cnd);
    mtx_unlock(&sankaku_mtx);

    mtx_lock(&pool_mtx);
    cnd_broadcast(&pool_cnd);
    mtx_unlock(&pool_mtx);

    for (size_t threads = 0; threads < THREADS; threads++)
        thrd_join(thrd_bot[threads], 0);

    thrd_join(thrd_sankaku, 0);

    debug_log(0, "init_bot: Done");

    cnd_destroy(&pool_flow);
    cnd_destroy(&pool_cnd);
    cnd_destroy(&sankaku_cnd);
    mtx_destroy(&pool_mtx);
    mtx_destroy(&sankaku_mtx);

    return 0;
}
