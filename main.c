#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Map Map;
typedef struct Key Key;
typedef struct Value Value;
typedef struct LinkedList LinkedList;
typedef struct AirportNode AirportNode;

struct Map
{
    Key **keys;
    Value **airports;
    int size;
    int airport_size;
    char *(*get)(Map *self, char *key);
    void (*add)(Map *self, char *key, char *name);
    void (*delete)(Map *self, char *key);
    void (*print_items)(Map *self);
};

struct Key
{
    char *key;
    int hash_code;
};

struct Value
{
    LinkedList *list;
};

struct LinkedList
{
    AirportNode *first;
    AirportNode *last;
    int size;
};

struct AirportNode
{
    AirportNode *next;
    AirportNode *previous;
    char *name;
    char *address;
    int key_index;
};

Map *initialize_map();
void release_map(Map *map);
Key *initialize_key(char *str_key, int hash_code);
Value *initialize_value(char *str_value, int key_index);
int generate_hash_code(char *value);
Value *get_value(Map *self, char *key);
AirportNode *get_node(Map *self, char *key);

char *get(Map *self, char *key);
void add(Map *self, char *key, char *name);
void delete(Map *self, char *key);
void print_items(Map *self);
void traverse_post_order_and_release_nodes(AirportNode *node);
AirportNode *traverse_and_get_node(Map *self, AirportNode *node, char *key);

int main()
{
    Map *map = initialize_map();

    map->add(map, "MCO", "Orlando International");
    map->add(map, "STI", "Aeropuerto Cibao");
    map->add(map, "SDQ", "Aeropuerto Las Americas");
    map->print_items(map);

    printf("\nget('MCO') = %s\n", map->get(map, "MCO"));
    printf("\nget('STI') = %s\n", map->get(map, "STI"));
    printf("\nget('SDQ') = %s\n", map->get(map, "SDQ"));

    printf("\ndelete('MCO')\n");
    map->delete (map, "MCO");
    map->print_items(map);
    printf("\ndelete('SDQ')\n");
    map->delete (map, "SDQ");
    map->print_items(map);

    release_map(map);

    return 0;
}

Map *initialize_map()
{
    Map *map = malloc(sizeof(Map));

    map->size = 0;
    map->airport_size = 0;
    map->keys = malloc(sizeof(Key));
    map->airports = malloc(sizeof(Value));

    map->get = &get;
    map->add = &add;
    map->delete = &delete;
    map->print_items = &print_items;

    return map;
}

void release_map(Map *map)
{
    for (int i = 0; i < map->size; i++)
    {
        Key *key = map->keys[i];
        Value *value = map->airports[key->hash_code];

        free(key);
        if (value->list->size > 0)
        {
            traverse_post_order_and_release_nodes(value->list->first);
        }
        free(value->list);
        free(value);
    }

    free(map->keys);
    free(map->airports);
    free(map);
}

void traverse_post_order_and_release_nodes(AirportNode *node)
{
    if (node == NULL)
    {
        return;
    }

    if (node->next != NULL)
    {
        traverse_post_order_and_release_nodes(node->next);
    }
    free(node);
}

Key *initialize_key(char *str_key, int hash_code)
{
    Key *key = malloc(sizeof(Key));

    key->key = str_key;
    key->hash_code = hash_code;

    return key;
}

Value *initialize_value(char *str_value, int key_index)
{
    Value *value = malloc(sizeof(Value));
    LinkedList *list = malloc(sizeof(LinkedList));
    AirportNode *node = malloc(sizeof(AirportNode));

    node->next = NULL;
    node->previous = NULL;
    node->name = str_value;
    node->key_index = key_index;

    list->size = 1;
    list->first = node;
    list->last = node;
    value->list = list;

    return value;
}

Value **realloc_and_initialize(Value **values, int old_size, int size)
{
    Value **new_values = realloc(values, size * sizeof(Value));

    for (int i = old_size; i < size; i++)
    {
        new_values[i] = NULL;
    }

    return new_values;
}

int generate_hash_code(char *key)
{
    int key_length = strlen(key);
    int hash_code_result = 0;

    for (int i = 0; i < key_length; i++)
    {
        char c = key[i];
        int ascii_code = (int)c;

        hash_code_result = hash_code_result + (ascii_code * i) + ascii_code;
    }

    return hash_code_result;
}

Value *get_value(Map *self, char *key)
{
    int hash_code = generate_hash_code(key);

    Value *value = hash_code > self->airport_size - 1 ? NULL : self->airports[hash_code];

    return value;
}

AirportNode *get_node(Map *self, char *key)
{
    int hash_code = generate_hash_code(key);

    Value *value = hash_code > self->airport_size - 1 ? NULL : self->airports[hash_code];

    if (value == NULL)
    {
        return NULL;
    }
    else
    {
        return traverse_and_get_node(self, value->list->first, key);
    }
}

AirportNode *traverse_and_get_node(Map *self, AirportNode *node, char *key)
{
    Key *original_key = self->keys[node->key_index];

    if (original_key->key == key)
    {
        return node;
    }
    else
    {
        if (node->next == NULL)
        {
            return NULL;
        }
        return traverse_and_get_node(self, node->next, key);
    }
}

// Time Complexity
// O(1)
// Space Complexity
// O(1)
char *get(Map *self, char *key)
{
    AirportNode *node = get_node(self, key);
    return node->name;
}

// Time Complexity
// O(1)
// Space Complexity
// O(1)
void add(Map *self, char *str_key, char *str_value)
{
    AirportNode *node = get_node(self, str_key);

    if (node == NULL)
    {
        int hash_code = generate_hash_code(str_key);
        int new_size = self->size + 1;
        int new_index = new_size - 1;

        Key *key = initialize_key(str_key, hash_code);

        self->keys = realloc(self->keys, new_size * sizeof(Key));
        self->keys[new_index] = key;
        self->size = new_size;

        if (hash_code > self->airport_size - 1)
        {
            Value *value = initialize_value(str_value, new_index);
            int new_values_size = hash_code + 1;
            self->airports = realloc_and_initialize(self->airports, self->airport_size, new_values_size);
            self->airports[hash_code] = value;
            self->airport_size = new_values_size;
        }
        else if (self->airports[hash_code] == NULL)
        {
            Value *value = initialize_value(str_value, new_index);
            self->airports[hash_code] = value;
        }
        else
        {
            Value *value = self->airports[hash_code];
            LinkedList *list = value->list;
            AirportNode *last_node = list->last;
            AirportNode *node = malloc(sizeof(AirportNode));

            last_node->next = node;

            node->next = NULL;
            node->previous = last_node;
            node->name = str_value;
            node->key_index = new_index;

            list->size = list->size + 1;
            list->last = node;
        }
    }
    else
    {
        node->name = str_value;
    }
}

// Time Complexity
// O(n)
// Space Complexity
// O(1)
void delete(Map *self, char *str_key)
{
    Value *value = get_value(self, str_key);
    AirportNode *node = get_node(self, str_key);
    Key *key = self->keys[node->key_index];

    for (int i = node->key_index + 1; i < self->size; i++)
    {
        int new_index = i - 1;
        if (self->keys[i]->key != str_key)
        {
            AirportNode *current_node = get_node(self, self->keys[i]->key);
            current_node->key_index = new_index;
        }
        self->keys[new_index] = self->keys[i];
    }

    if (value->list->size > 1)
    {
        if (value->list->last == node)
        {
            AirportNode *previous_node = node->previous;
            previous_node->next = NULL;
            value->list->last = previous_node;
        }
        else if (value->list->first == node)
        {
            AirportNode *next_node = node->next;
            next_node->previous = NULL;
            value->list->first = next_node;
        }
        else
        {
            AirportNode *previous_node = node->previous;
            previous_node->next = node->next;
        }
        value->list->size = value->list->size - 1;
        free(node);
    }
    else
    {
        free(node);
        free(value->list);
        free(value);
        self->airports[key->hash_code] = NULL;
    }

    free(key);
    int new_size = self->size - 1;
    self->keys = realloc(self->keys, new_size * sizeof(Key));
    self->size = new_size;
}

// Time Complexity
// O(n)
// Space Complexity
// O(1)
void print_items(Map *self)
{
    printf("\n[");

    for (int i = 0; i < self->size; i++)
    {
        Key *key = self->keys[i];
        AirportNode *node = get_node(self, key->key);

        printf("{%s, %s} ", key->key, node->name);
    }

    printf("]\n");
}