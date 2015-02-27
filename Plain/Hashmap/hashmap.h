#ifndef PLAIN_HASHMAP_H_
#define PLAIN_HASHMAP_H_

struct _PlainHashmapEntry{
	struct _PlainHashmapEntry *next;
	void *key;
	void *value;
};
typedef struct _PlainHashmapEntry PlainHashmapEntry;

//Should return 0 if not equal
typedef int (*PlainHashmapKeyComparator)(void *key1, void *key2);
typedef int (*PlainHashmapHasher)(void *key);

typedef struct{
	PlainHashmapKeyComparator comparator;
	PlainHashmapHasher hasher;
	int keySize, valueSize, height;
	PlainHashmapEntry **entries;
}PlainHashmap;


PlainHashmap *plainHashMapInit(PlainHashmapKeyComparator comparator,
				PlainHashmapHasher hasher,
				int keySize, int valueSize, int height);

int plainHashMapInsert(PlainHashmap *map, void *key, void *value);
void *plainHashMapGet(PlainHashmap *map, void *key);
void plainHashMapDelete(PlainHashmap *map, void *key);

void plainHashMapDeleteEntries(PlainHashmap *map);

void plainHashMapFree(PlainHashmap *map);

#endif
