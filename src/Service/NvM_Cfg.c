/* NvM_Cfg.c - block descriptors, RAM and ROM blocks */
#include "NvM.h"
#include "Eep.h"

NvM_Block_OdometerType NvM_RamBlock_Odometer;

static const NvM_Block_OdometerType NvM_RomBlock_Odometer = { 0U, 0U };

const NvM_BlockDescriptorType NvM_BlockDescriptor[NVM_NUM_USER_BLOCKS] =
{
    /* NvAddress copy 0 / copy 1, Length, RAM block, ROM defaults */
    { { EEP_NVM_COPY0_ADDR, EEP_NVM_COPY1_ADDR }, (uint8)sizeof(NvM_Block_OdometerType), &NvM_RamBlock_Odometer, &NvM_RomBlock_Odometer },
};
