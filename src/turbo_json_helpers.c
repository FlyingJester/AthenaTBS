#include "turbo_json_helpers.h"
#include <TurboJSON/object.h>
#include <string.h>

static long turbo_helper_get_object_element_iter(
    const struct Turbo_Property *object, const unsigned length, 
    const unsigned i,
    const char *name, const unsigned name_len){

    if(!length)
        return -1;
    else if(name_len == object->name_length && memcmp(object->name, name, name_len)==0){
        return i;
    }
    else{
        return turbo_helper_get_object_element_iter(object+1, length-1, i+1, name, name_len);
    }
}

struct Turbo_Value *Turbo_Helper_GetObjectElement(struct Turbo_Value *from, const char *name){
    return (struct Turbo_Value *)Turbo_Helper_GetConstObjectElement(from, name);
}

const struct Turbo_Value *Turbo_Helper_GetConstObjectElement(const struct Turbo_Value *from, const char *name){
    if(from->type==TJ_Object){
        const int i = turbo_helper_get_object_element_iter(from->value.object, from->length, 0, name, strlen(name));
        if(i>=0){
            return &from->value.object[i].value;
        }
    }
    return NULL;
}

unsigned Turbo_Helper_CompareString(const struct Turbo_Value *str_value, const char *with, unsigned len){
    if(str_value->type!=TJ_String)
        return 0;
    if(str_value->length != len)
        return 0;
    return memcmp(with, str_value->value.string, len)==0;
}
