/* MemIf_Types.h - memory stack status types shared by Eep and NvM */
#ifndef MEMIF_TYPES_H
#define MEMIF_TYPES_H

typedef enum
{
    MEMIF_UNINIT = 0U,
    MEMIF_IDLE,
    MEMIF_BUSY
} MemIf_StatusType;

typedef enum
{
    MEMIF_JOB_OK = 0U,
    MEMIF_JOB_FAILED,
    MEMIF_JOB_PENDING
} MemIf_JobResultType;

#endif /* MEMIF_TYPES_H */
