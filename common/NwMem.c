/*----------------------------------------------------------------------------*
 *                                                                            *
 * Copyright (c) 2010-2011 Amit Chawre                                        *
 * All rights reserved.                                                       *
 *                                                                            *
 * Redistribution and use in source and binary forms, with or without         *
 * modification, are permitted provided that the following conditions         *
 * are met:                                                                   *
 *                                                                            *
 * 1. Redistributions of source code must retain the above copyright          *
 *    notice, this list of conditions and the following disclaimer.           *
 * 2. Redistributions in binary form must reproduce the above copyright       *
 *    notice, this list of conditions and the following disclaimer in the     *
 *    documentation and/or other materials provided with the distribution.    *
 * 3. The name of the author may not be used to endorse or promote products   *
 *    derived from this software without specific prior written permission.   *
 *                                                                            *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR       *
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES  *
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.    *
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,           *
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT   *
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  *
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY      *
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT        *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF   *
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.          *
 *----------------------------------------------------------------------------*/


/** 
 * @file NwMem.h 
 * @brief This header file contains memory related operations definitions.
 */

#include "NwMem.h"
#include "NwUtils.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NW_MEM_START_MAGIC      (0x12345678)
#define NW_MEM_END_MAGIC        (0x87654321)

#define NW_MEM_STATUS_FREE      (0)
#define NW_MEM_STATUS_ALLOCATED (1)

#define NW_MEM_MAX_SIZE         (20 * 1024)

typedef struct NwMemChunkS
{
  NwU32T                status;
  struct NwMemChunkS    *pNext;
} NwMemChunkT;

static
NwMemChunkT* memPool[NW_MEM_MAX_SIZE];

NwRcT
nwMemInitialize()
{
  NwU32T i;
  for(i = 0; i < NW_MEM_MAX_SIZE; i ++)
  {
    memPool[i] = NULL;
  }
  return NW_OK;
}

NwRcT
nwMemFinalize()
{
  return NW_OK;
}

void*
_nwMemNew(NwU32T size, NwCharT* fn, NwU32T ln)
{
  NwMemChunkT* memChunk;
  void* mem;

  //printf("\nRequest for new mem of size %u from %s:%u\n", size, fn, ln);
  if(size < NW_MEM_MAX_SIZE)
  {
    if(memPool[size] == NULL)
    {
      memChunk = malloc (sizeof(NwMemChunkT) + 4 + 4 + size + 4);
      NW_ASSERT(memChunk);
      memChunk->pNext = NULL;
      mem = (NwU8T*) memChunk + sizeof(NwMemChunkT);

      *((NwU32T*)mem)                             = NW_MEM_START_MAGIC;
      *((NwU32T*)(((NwU8T*) mem) + 4))            = size;
      *((NwU32T*)(((NwU8T*) mem) + 8 + size ))    = NW_MEM_END_MAGIC;

      //printf("Allocating new mem 0x%x of size %u from %s:%u\n", mem, size, fn, ln);
    }
    else
    {
      memChunk = memPool[size];
      memPool[size] = memPool[size]->pNext;
      mem = (NwU8T*) memChunk + sizeof(NwMemChunkT);
      //printf("Allocating existing mem 0x%x of size %u from %s:%u\n", mem, size, fn, ln);
    }

    if(memChunk->status == NW_MEM_STATUS_ALLOCATED)
    {
      //printf("\nRequest for new mem of size %u from %s:%u failed!\n", size, fn, ln);
      NW_ASSERT(memChunk->status  != NW_MEM_STATUS_ALLOCATED);
    }
    memChunk->status = NW_MEM_STATUS_ALLOCATED;

    return (((NwU8T*) mem) + 8) ;
  }
  else
  {
    return NULL;
  }
}

void
_nwMemDelete(void* hMem, NwCharT* fn, NwU32T ln)
{
  void* mem;
  NwU32T size;
  NwMemChunkT* memChunk;

  mem = (NwU8T*) hMem - 8;
//  printf("Free memory 0x%x request of size %u from %s:%u\n", mem, *((NwU32T*)(((NwU8T*) mem) + 4)), fn, ln);

  NW_ASSERT(*((NwU32T*)mem) == NW_MEM_START_MAGIC);

  size = *((NwU32T*)(((NwU8T*) mem) + 4));
  NW_ASSERT(size < NW_MEM_MAX_SIZE);
  NW_ASSERT(*((NwU32T*)(((NwU8T*) mem) + 8 + size )) == NW_MEM_END_MAGIC);

  memChunk = mem - sizeof(NwMemChunkT);
  if(memChunk->status != NW_MEM_STATUS_ALLOCATED)
  {
    //printf("Free memory 0x%x request of size %u from %s:%u failed!\n", mem, *((NwU32T*)(((NwU8T*) mem) + 4)), fn, ln);
    NW_ASSERT(memChunk->status);
  }

  memChunk->pNext = memPool[size];
  memPool[size] = memChunk;
  memChunk->status = NW_MEM_STATUS_FREE;
}

#ifdef __cplusplus
}
#endif


