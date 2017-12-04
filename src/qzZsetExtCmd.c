/* 
 * Module designed to extent the zset opration command.
 */

#include "../redismodule.h"
#include <stdlib.h>

int QzZpopif(RedisModuleCtx *ctx, RedisModuleString **argv, int argc, int reverse) {
    /* 检查参数 */
    if (argc < 2 || argc > 3) {
        return RedisModule_WrongArity(ctx);
    }
    /* 获取可选参数 */
    int ifMode = 0;
    double ifThreshold = 0;
    if (argc == 3) {
        ifMode = 1;
        if (REDISMODULE_OK != RedisModule_StringToDouble(argv[2], &ifThreshold)) {
            return RedisModule_ReplyWithError(ctx, "ERR Wrong type of arg[2]");
        }
    }
    
    /* 使用自动内存管理，简化key的close */
    RedisModule_AutoMemory(ctx);
    
    /* 获取key，并判断是否存在或为ZSET */
    RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    if (RedisModule_KeyType(key) == REDISMODULE_KEYTYPE_EMPTY) {
        RedisModule_ReplyWithNull(ctx);
        return REDISMODULE_OK;
    } else if (RedisModule_KeyType(key) != REDISMODULE_KEYTYPE_ZSET) {
        RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        return REDISMODULE_ERR;
    }
    
    /* 找到要弹出的元素 */
    int ret = REDISMODULE_OK;
    if (reverse) {
        ret = RedisModule_ZsetFirstInScoreRange(key, REDISMODULE_NEGATIVE_INFINITE, REDISMODULE_POSITIVE_INFINITE, 0, 0);
    } else {
        ret = RedisModule_ZsetLastInScoreRange(key, REDISMODULE_NEGATIVE_INFINITE, REDISMODULE_POSITIVE_INFINITE, 0, 0);
    }
    if (ret) {
        RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE); /* 前面检查过类型，正常情况这里不可能出错 */
        return REDISMODULE_ERR;
    }
    
    double score = 0;
    RedisModuleString *ele = RedisModule_ZsetRangeCurrentElement(key, &score);
    RedisModule_ZsetRangeStop(key); /* 停止迭代器 */
    if (!ele) {
        RedisModule_ReplyWithNull(ctx);
        return REDISMODULE_OK;
    }
    
    /* 检查是否要pop元素 */
    int delete = 0;
    if (ifMode) {
        if (reverse) {
            if (score <= ifThreshold) delete = 1;
        } else {
            if (score >= ifThreshold) delete = 1;
        }
    } else {
        delete = 1;
    }
    
    /* 删除元素并返回其值，或不删除返回nil */
    if (delete) {
        ret = RedisModule_ZsetRem(key, ele, NULL);
        if (ret) {
            RedisModule_ReplyWithError(ctx, "ZsetRem ERROR");
            return REDISMODULE_ERR;
        }        
        RedisModule_ReplyWithArray(ctx, 2);
        RedisModule_ReplyWithString(ctx, ele);
        RedisModule_ReplyWithDouble(ctx, score);
        
        /* 产生写操作时，复制命令到AOF和从机 */
        RedisModule_ReplicateVerbatim(ctx);
        return REDISMODULE_OK;
    }
    RedisModule_ReplyWithNull(ctx);
    return REDISMODULE_OK;
}

int QzZlpopif_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    return QzZpopif(ctx, argv, argc, 1);
}

int QzZrpopif_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    return QzZpopif(ctx, argv, argc, 0);
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    REDISMODULE_NOT_USED(argv);
    REDISMODULE_NOT_USED(argc);

    if (RedisModule_Init(ctx,"qzZsetExtCmd",1,REDISMODULE_APIVER_1)
        == REDISMODULE_ERR) return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"qz.zlpopif",
        QzZlpopif_RedisCommand,"write deny-oom",1,1,1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"qz.zrpopif",
        QzZrpopif_RedisCommand,"write deny-oom",1,1,1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    return REDISMODULE_OK;
}
