#include "Baal.h"
#include "todo.h"


void BaalIterator_init(BaalIterator* iter, const Baal* baal) {
    iter->current = NULL;

    iter->__internal.baal = baal;
}

void* BaalIterator_next(BaalIterator* iter) {
    if(iter->current == NULL) {
        Baal_internal_ChunkInfo* chunk = (Baal_internal_ChunkInfo*)iter->__internal.baal->buffer;

        if(iter->__internal.baal->first == chunk) {
            iter->isFree = 1;
            iter->__internal.end = chunk->nextChunk;
        } else {
            iter->isFree = 0;
            iter->__internal.end = iter->__internal.baal->first;
        }

        iter->index = 0;
        iter->current = (char*)chunk + BAAL_ADDED_INFO_SIZE;
        iter->chunkSize = chunk->chunkSize;
        return iter->current;
    }

    Baal_internal_ChunkInfo* current = (Baal_internal_ChunkInfo*)((char*)iter->current - BAAL_ADDED_INFO_SIZE);

    Baal_internal_ChunkInfo* next = (Baal_internal_ChunkInfo*)((char*)current + current->chunkSize * iter->__internal.baal->groupLength);

    if((void*)next == (void*)(iter->__internal.baal->buffer + iter->__internal.baal->groupsNumber * iter->__internal.baal->groupLength)) {
        return NULL;
    }

    if(next == iter->__internal.end) {
        iter->isFree = 1;
        
        if(next->nextChunk) {
            iter->__internal.end = next->nextChunk;
        } else {
            iter->__internal.end = (void*)(iter->__internal.baal->buffer + iter->__internal.baal->groupsNumber * iter->__internal.baal->groupLength);
        }
    } else {
        iter->isFree = 0;
    }

    iter->index += current->chunkSize;
    iter->current = (char*)next + BAAL_ADDED_INFO_SIZE;
    iter->chunkSize = next->chunkSize;

    return iter->current;
}

void BaalIterator_freeCurrent(BaalIterator* iter) {
    Baal_free((Baal*)iter->__internal.baal, iter->current);
}
