/* NvM_Cfg.h - NvM blocks of the cluster */
#ifndef NVM_CFG_H
#define NVM_CFG_H

#include "Std_Types.h"

#define NVM_FIRST_USER_BLOCK          2U    /* block 0 = multi block, 1 = config ID (not used) */
#define NvMConf_NvMBlock_Odometer     2U
#define NVM_NUM_USER_BLOCKS           1U
#define NVM_MAX_BLOCK_LENGTH          16U

typedef struct
{
    uint32 OdometerM;   /* total distance in metres */
    uint32 TripM;       /* trip distance in metres */
} NvM_Block_OdometerType;

extern NvM_Block_OdometerType NvM_RamBlock_Odometer;

#endif /* NVM_CFG_H */
