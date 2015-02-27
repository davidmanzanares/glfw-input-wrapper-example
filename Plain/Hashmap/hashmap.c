#include <stdlib.h>
#include <string.h>
#include "hashmap.h"


PlainHashmap *plainHashMapInit(PlainHashmapKeyComparator comparator,
							   PlainHashmapHasher hasher,
							   int keySize, int valueSize, int height){
	PlainHashmap *map=malloc(sizeof(PlainHashmap));
	if (!map){
		return NULL;
	}
	map->entries=calloc(height, sizeof(PlainHashmapEntry*));
	if (!map->entries){
		free(map);
		return NULL;
	}
	map->keySize=keySize;
	map->valueSize=valueSize;
	map->height=height;
	map->comparator=comparator;
	map->hasher=hasher;
	return map;
}

int plainHashMapInsert(PlainHashmap *map, void *key, void *value){
	//Alloc memory for key and value
	void *entryKey=malloc(map->keySize);
	void *entryValue=malloc(map->valueSize);
	if (!entryKey || !entryValue){
		return -1;
	}
	//Copy key and value values
	memcpy(entryKey, key, map->keySize);
	memcpy(entryValue, value, map->valueSize);

	//Get dest entry
	int h=map->hasher(key)%map->height;
	PlainHashmapEntry *entry=map->entries[h];
	if (entry){
		//Collision detected: use the linked list
		while (entry->next){
			entry=entry->next;
		}
		//entry is the last valid entry of the linked list
		//Alloc new entry
		entry->next=malloc(sizeof(PlainHashmapEntry));
		if (!entry->next){
			free(entryKey);
			free(entryValue);
			return -1;
		}
		//Point to the new allocated entry
		entry=entry->next;
	}else{
		map->entries[h]=malloc(sizeof(PlainHashmapEntry));
		entry=map->entries[h];
	}
	//Set entry params
	entry->next=NULL;
	entry->key=entryKey;
	entry->value=entryValue;
	return 0;
}
void *plainHashMapGet(PlainHashmap *map, void *key){
	int h=map->hasher(key)%map->height;
	for (PlainHashmapEntry *entry=map->entries[h]; entry; entry=entry->next){
		if (map->comparator(entry->key, key)){
			//Match found
			return entry->value;
		}
	}
	//Not found
	return NULL;
}
void plainHashMapDelete(PlainHashmap *map, void *key){
	//Get dest entry
	int h=map->hasher(key)%map->height;
	PlainHashmapEntry *entry=map->entries[h];
	if (!entry){
		return;
	}
	//Check first
	if (map->comparator(entry->key, key)){
		//Match found
		map->entries[h]=entry->next;
		free(entry->key);
		free(entry->value);
		free(entry);
		return;
	}
	//Iterate linked list
	while (entry->next){
		if (map->comparator(entry->next->key, key)){
			//Match found
			entry->next=entry->next->next;
			free(entry->next->key);
			free(entry->next->value);
			free(entry->next);
			return;
		}
		entry=entry->next;
	}
}
void plainHashMapDeleteEntries(PlainHashmap *map){
	for (int i=0; i<map->height; i++){
		if (map->entries[i]){
			PlainHashmapEntry *nextEntry=map->entries[i];
			while (nextEntry){
				PlainHashmapEntry *entry=nextEntry;
				nextEntry=nextEntry->next;
				free(entry->key);
				free(entry->value);
				free(entry);
			}
			map->entries[i]=NULL;
		}
	}
}

void plainHashMapFree(PlainHashmap *map){
	plainHashMapDeleteEntries(map);
	free(map->entries);
	free(map);
}

