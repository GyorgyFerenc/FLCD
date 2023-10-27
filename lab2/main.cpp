#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <iostream>
#include <string>

#define usize size_t
#define let   auto
using u8 = uint8_t;

template <class T>
T* typed_malloc(usize size){
    return (T*)malloc(sizeof(T) * size);
}

template <class T>
T* typed_realloc(T* ptr, usize size){
    return (T*)realloc(ptr, sizeof(T) * size);
}


template <class T>
void typed_memset(T* ptr, T value, usize len){
    for (usize i = 0; i < len; i++){
        ptr[i] = value;
    }
}


struct Hash_Table{
    struct Key{
        usize index = 0;
        enum class Kind: u8{
            Free = 0,
            Used,
        } kind = Kind::Free;
    };
    struct Value{
        std::string str;
    };

    usize   capacity;
    usize   size;
    Key*    keys;
    Value*  values;

    static Hash_Table create(usize capacity){
        Hash_Table table;
        table.capacity  = capacity;
        table.size      = 0;
        table.keys      = new Key[capacity]; 
        table.values    = new Value[capacity];
        return table;
    }

    void destroy(){
        delete[] this->keys;
        delete[] this->values;
    }

    usize add(std::string str){
        if (this->size + 1 >= this->capacity){
            recapacity(this->capacity * 2);
        }

        let key_index = hash_to_key_index(str, this->keys, this->values, this->capacity);
        let value_index = this->keys[key_index].index;
        if (this->values[value_index].str == str){
            return value_index;
        }

        let value_pos = this->size;
        this->size++;
        this->values[value_pos].str = str;
        this->keys[key_index].index = value_pos;
        this->keys[key_index].kind  = Key::Kind::Used;

        return value_pos;
    }

    std::string get(usize index){
        return this->values[index].str;       
    }

private:
    usize hash_to_key_index(std::string str, Key* keys, Value* values, usize capacity){
        /*
         * Currently does linear probing
         */
        let key_index = this->hash(str) % capacity;
        let i = 0;
        while (keys[key_index].kind != Key::Kind::Free){
            let index = keys[key_index].index;
            if (values[index].str == str) return key_index;

            i++;
            key_index = (this->hash(str) + this->prob(i)) % capacity;
        }
        return key_index;
    }

    void recapacity(usize new_capacity){
        let new_keys   = new Key[new_capacity];
        let new_values = new Value[new_capacity];

        for (usize i = 0; i < this->size; i++)
            new_values[i] = this->values[i];

        for (usize i = 0; i < this->capacity; i++){
            let key = this->keys[i];
            if (key.kind == Key::Kind::Used){
                let key_index = hash_to_key_index(new_values[key.index].str, new_keys, new_values, new_capacity);
                new_keys[key_index] = key;
            }
        }
        
        delete[] this->values;
        delete[] this->keys;
        this->values   = new_values;
        this->keys     = new_keys;
        this->capacity = new_capacity;
    }


    inline
    usize hash(std::string str){
        std::hash<std::string> hasher;
        return hasher(str);
    }
    inline
    usize prob(usize i){
        return i;
    }

};

struct Symbol_Table{
    Hash_Table identifiers;
    Hash_Table constants;
    
    static Symbol_Table create(usize capacity){
        return {
            .identifiers = Hash_Table::create(capacity),
            .constants   = Hash_Table::create(capacity),
        };
    }
};

void hash_table_test(){
    let table = Hash_Table::create(10);
    let check = [&table](std::string str){
        let index = table.add(str);
        assert(table.get(index) == str);
    };
    check("Kecske");
    check("Kecske");
    check("Kecske");
    check("Kecske");
    check("Kecske");
    check("Kecske");
    check("Kecske");
    check("Kecske");
    check("Kecske");
    check("Kecske");
    check("Kecske");
    check("Kecske");
    check("Kecske");
    check("Kecske");
    check("Kecske");
    check("Kecske");
    check("Kecske");
    check("ment");
    check("a");
    check("kis");
    check("kertbe");

    table.destroy();
}


void hash_table_test2(){
    let table = Hash_Table::create(1);
    let check = [&table](std::string str){
        let index = table.add(str);
        assert(table.get(index) == str);
        return index;
    };
    let first  = check("Kecske");
    let second = check("Kecske");
    check("Kecske");
    check("Kecske");
    check("ment");
    check("Kecske");
    check("Kecske");
    check("Kecske");
    check("Kecske");
    check("ment");
    check("ment");
    check("a");
    check("kis");
    check("kertbe");
    let third = check("Kecske");
    check("Kecske");
    check("Kecske");
    check("Kecske");

    // first should be valid even if the table growths
    assert(table.get(first) == "Kecske");
    assert(first == second);
    assert(first == third);
    assert(second == third);

    table.destroy();
}

int main(){
    hash_table_test();
    hash_table_test2();
}
