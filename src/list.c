/**
 * List file holding the functionalities necessary for a dynamic list (not linked list).
 * 
 * Author: Fabio Hux
 * 
 * Date Created: 5/24/2020
 * 
 * Date Last Edited: 7/5/2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "list.h"

void insertionResize(List *list);
void shiftOneRight(char *start, long offset, char *stop);
void shiftOneLeft(char *start, long offset, char *stop);

/**
 * Function to compare to elements in a list by int types.
 * 
 * Function will return a value equating to *a - *b in the forms of integers. Function is to be used as cmp for createDataList() in the event the elements are of type int.
 * 
 */
int intCmp(void *a, void *b){
    return *((int *)a) - *((int *)b);
}

/**
 * Function to compare to elements in a list by double types.
 * 
 * Function will return a int value equating to double casted *a - *b. Function is to be used as cmp for createDataList() in the event the elements are of type double.
 * 
 */
int dblCmp(void *a, void *b){
    return (int) ((*((double *)a) - *((double *)b)));
}

/**
 * 
 */
void listDestroyer(void *element){
    if(element == NULL) return;

    List *list = *((List **) element);

    deleteDataList(list);
}

/**
 * Function to append an element to the end of the list.
 * 
 * Function will add the element to the end of the list, adjusting the size of the list if necessary.
 * 
 * Will return a 0 or 1 on failure or success, respectively. Function will exit the program in the event that it cannot reallocate space for the list.
 */
char append(List *list, void *value){
    if(list->size == list->len){
        insertionResize(list);
    }

    return set(list, list->size++, value);
}

/**
 * Function to shift certain elements in a list to the right to make room for an insert.
 * 
 * Used as a helper function for insert(), the function will move all the data from stop to the right of the list until it reaches start. (start > stop). Offset must be the list's eSize value.
 * 
 * NOTE: There is no safety in this function and must be done with care!
 */
void shiftOneRight(char *start, long offset, char *stop){
    asm volatile(
        "jmp 2f\n\t"
        "1:movq (%0), %%rcx\n\t"
        "movq %%rcx, (%0, %1)\n\t"
        "subq $8, %0\n\t"
        "2: cmpq %2, %0\n\t"
        "jge 1b\n\t"
        "addq $7, %0\n\t"
        "jmp 4f\n\t"
        "3:movb (%0), %%cl\n\t"
        "movb %%cl, (%0, %1)\n\t"
        "decq %0\n\t"
        "4: cmpq %2, %0\n\t"
        "jge 3b\n\t"
        : 
        : "D" (start), "S" (offset), "d" (stop)
        : "%rcx", "%cl"
    );
}

/**
 * Function to shift certain elements in a list to the left to remove an element.
 * 
 * Used as a helper function for removeRet(), the function will move all the data from start to the left of the list until it reaches stop. (start < stop). Offset must be the list's eSize value.
 * 
 * NOTE: There is no safety in this function and must be done with care!
 */
void shiftOneLeft(char *start, long offset, char *stop){
    asm volatile(
        "jmp 2f\n\t"
        "1:movq (%0, %1), %%rcx\n\t"
        "movq %%rcx, (%0)\n\t"
        "addq $8, %0\n\t"
        "2: cmpq %2, %0\n\t"
        "jl 1b\n\t"
        "subq $7, %0\n\t"
        "jmp 4f\n\t"
        "3:movb (%0, %1), %%cl\n\t"
        "movb %%cl, (%0)\n\t"
        "incq %0\n\t"
        "4: cmpq %2, %0\n\t"
        "jl 3b\n\t"
        : 
        : "D" (start), "S" (offset), "d" (stop)
        : "%rcx", "%cl"
    );
}

/**
 * Function to insert an element to a list at a specific index.
 * 
 * Function will insert a given element to the list, adjusting the size of the list if necessary.
 * 
 * Will return a 0 or 1 on failure or success, respectively. Function will exit the program in the event that it cannot reallocate space for the list.
 */
char insert(List *list, void *value, int index){
    if(index > list->size || index < 0){
        return 0;
    }else if(index == list->size){
        return append(list, value);
    }else{
        if(list->size == list->len){
            insertionResize(list);
        }

        //Previous implementation. While good, calling set continuously or shifting byte by byte proved to be slow.
        /*char *stop = (char *) list->data + (index * list->eSize);
        char *prev = (char *) list->data + (list->size++ * list->eSize) - 1;
        char *curr = (char *) list->data + (list->size * list->eSize) - 1;
        while(prev >= stop){
            *(curr--) = *(prev--);
            //set(list, i, list->data + ((i - 1)*list->eSize));
        }*/
        
        char *stop = (char *) list->data + ((index) * list->eSize);
        char *start = (char *) list->data + ((list->size++) * list->eSize) - 8;
        shiftOneRight(start, list->eSize, stop);

        return set(list, index, value);
    }
}

/**
 * Function to insert an element to the list at the location that will keep the list sorted.
 * 
 * Function is identical to insert() (see insert()) with the exception that it will choose the index to insert the element to by using the cmpVal() function (see cmpVal()).
 * 
 * Will return a 0 or 1 on failure or success, respectively, Function will exit the program in the event that it cannot reallocate space for the list.
 */
char insertSorted(List *list, void *value){
    int index = closestIndexOf(list, value);

    return insert(list, value, index);
}

/**
 * Function used to adjust the size of the list if there is insufficient space.
 * 
 * Function will compare the size and the length and in the event that both values are the same, it will realloc the list to twice the length.
 * 
 * NOTE: Should not be used by outside user, rather a helper function for append(), insert(), and insertSorted().
 */
void insertionResize(List *list){
    if(list->size == list->len){
        list->data = (char *)realloc(list->data, list->eSize * (list->len * 2));
        if(list->data == NULL){
            printf("An error occurred with realloc in insertionResize. Exiting.\n");
            exit(0);
        }else{
            list->len <<= 1;
        }
    }
}

/**
 * Function to remove an element from the list at a given index (if valid), returning the removed element.
 * 
 * Function will adjust the list to close the gap made by the removal of a selected element, and return the element removed.
 * 
 * Function will return NULL in the event that an invalid index was returned or if there is a failure in removal.
 * NOTE: The function will call malloc() to allocate space for the returned pointer and will need to be freed by the given user.
 * 
 * EXAMPLE: List of ints: {0, 1, 2, 3, 4}. Element at index 2 is removed. Return: void * -> [2], Updated List: {0, 1, 3, 4}. How to access the value: dereference given pointer.
 */
void *removeRet(List *list, int index){
    if(index < 0 || index >= list->size){
        return NULL;
    }

    void *value = calloc(1,list->eSize);

    if(value == NULL){
        printf("An error occurred with malloc in remove. Exiting.\n");
        exit(0);
    }

    char *start = (char *) list->data + (index * list->eSize);

    memcpy(value, start, list->eSize);

    if(index < list->size - 1){
        char *stop = (char *) list->data + ((--list->size) * list->eSize);

        shiftOneLeft(start, list->eSize, stop);
    }else{
        --list->size;
    }

    //list->size--;
    return value;
}

/**
 * Function to remove an element from the list at a given index (if valid), returning the removed element.
 * 
 * Function is identical to removeRet() (see removeRet()) with the exception that the return value will not be the element, rather a 0 or 1 indicating the failure or success, respectively.
 * 
 * NOTE: In the case that the list is of strings or lists, it will be freed by the function.
 */
/*int removeNoRet(List *list, int index){
    if(index < 0 || index >= list->size) return 0;

    if(list->dType == 5){
        free(*((char **) list->data + index));
    }else if(list->dType == 6){
        deleteDataList(*((List **) list->data + index));
    }

    int i = index;
    while(i < list->size - 1){
        set(list, i, list->data + (i + 1));
        ++i;
    }

    list->size--;
    return 1;
}*/

/**
 * Function to get an element from the list at a given index (if valid).
 * 
 * Function will return the element at a specific index on success or a NULL on failure. The return will be a pointer of the stored data type in the form of a void *.
 * 
 * EXAMPLE: A list of int types would return an int * in the form of a void *. A list of list types would return a List ** in the form of a void *.
 * 
 * NOTE: Function is dangerous as it is a direct access of the data list and any manipulation would result in the manipulation of data of the given list.
 */
void *get(List *list, int index){
    if(index < 0 || index >= list->size){
        return NULL;
    }

    return list->data + (index * list->eSize);
}

/**
 * Function to set an element from the list at a given index (if valid) to a specific given value.
 * 
 * Function will return a 0 or 1 on the failure or success of the operation, respectively.
 * NOTE: In the case the list is that of strings or nested lists, performing this function will free the memory of the original values before setting it to the new value.
 */
char set(List *list, int index, void *value){
    if(index >= list->size || index < 0){
        return 0;
    }

    memcpy(list->data + (index * list->eSize), value, list->eSize);
    
    return 1;
}

/**
 * Function to find the index of a given value in the list.
 * 
 * Function will return an integer indicating the index in which the given value is present. In the case it does not exist in the list, it will return -1.
 * 
 * NOTE: This function uses binary search and not having a sorted list will lead to errors. It is suggested that a manual iteration is done to search for the index instead.
 * In this case, it would mean that if one were to search for the index of a List, it would attempt to search it by the address of the pointer.
 */
int indexOf(List *list, void *value){
    /*int index = closestIndexOf(list, value);
   
    if(index >= list->size || (list->cmp == NULL && memcmp(list->data + (index * list->eSize), value, list->eSize)) || list->cmp(list->data + (index * list->eSize), value)){
        index = -1;
    }

    return index;*/

    if(list->size == 0){
        return -1;
    }

    int low = 0, high = list->size - 1, mid, diff;

    while(1){
        diff = cmpVal(list->data + (low * list->eSize), value, list);
        if(diff > 0){
            return -1;
        }else if(!diff){
            return low;
        }else{
            diff = cmpVal(list->data + (high * list->eSize), value, list);
            if(diff < 0){
                return -1;
            }else if(!diff){
                return high;
            }else{
                mid = (high + low) >> 1;
                diff = cmpVal(list->data + (mid * list->eSize), value, list);
                if(!diff){
                    return mid;
                }else if(diff < 0){
                    low = mid + 1;
                }else{
                    high = mid - 1;
                }
            }
        }
    }
}

/**
 * Function to find the index of a given value in the list or the index with which it is preferable to insert the given value to the list.
 * 
 * Function is identical in use as indexOf() (see indexOf()) with the exception that instead of return -1 on failure for searching, it will instead return the index to potentially insert the given value into the given list.
 * Normally it should not provide any functional use to an outside user and it is meant to be used as a helper function.
 * 
 * NOTE that this function uses binary search and not having a sorted list will lead to errors.
 */
int closestIndexOf(List *list, void *value){
    if(list->size == 0){
        return 0;
    }

    int low = 0, high = list->size - 1, mid, diff;

    while(1){
        if(cmpVal(list->data + (low * list->eSize), value, list) > 0){
            return low;
        }else if(cmpVal(list->data + (high * list->eSize), value, list) < 0){
            return high + 1;
        }else{
            mid = (high + low) >> 1;
            diff = cmpVal(list->data + (mid * list->eSize), value, list);
            if(!diff){
                return mid;
            }else if(diff < 0){
                low = mid + 1;
            }else{
                high = mid - 1;
            }
        }
    }
}

/**
 * Function to compare the values of two given inputs, returning a numerical value representing its comparative value.
 * 
 * Function will return a 0 if the values in which a and b are pointing to are identical in value.
 * Function will return a negative number if the values in which a and b are pointing to result in *b > *a.
 * Function will return a positive number if the values in which a and b are pointing to result in *a > *b.
 * 
 * NOTE: In the case the list is of strings, it will use the strcmp() function to compare the strings.
 * NOTE: In the case the list is of lists, it will compare the pointer values of each List to determine identicality.
 */
int cmpVal(void *a, void *b, List *list){
    if(list->cmp == NULL){
        return memcmp(a, b, list->eSize);
    }else{
        return list->cmp(a, b);
    }
}

/**
 * Function to create a list of a given length and certain type.
 * 
 * Function will create a list to return to the caller that is built to the specific length and type which the user has requested for.
 * NULL will be returned in the case that: A)The len given is <= 0. B)The type given does not follow one of the given types in the list ["int", "long", "float", "double", "char", "string", "list"].
 * 
 * NOTE: Function will cause an exit in the case mallocing the list or the data results in a NULL pointer.
 */
List *createDataList(int len, size_t size, int (*cmp)(void *, void *), void (*destroy)(void *)){
    if(len <= 0) return NULL;

    List *list = (List *) calloc(1,sizeof(List));

    if(list == NULL){
        printf("An error occurred with malloc in createDataList on creating the list. Exiting.\n");
        exit(0);
    }

    list->data = (char *) calloc(len, size);

    if(list->data == NULL){
        printf("An error occurred with malloc in createDataList on setting the data list. Exiting.\n");
        exit(0);
    }

    list->size = 0;
    list->len = len;
    list->eSize = size;
    list->cmp = cmp;
    list->destroy = destroy;

    return list;
}

/**
 * Function to delete a given list.
 * 
 * Function will clean all the inner data before freeing the list provided.
 * 
 * NOTE: For the case of the list being of type string or list, the function will free each string or call deleteDataList for each list within the list, respectively.
 */
void deleteDataList(List *list){
    if(list == NULL) return;

    char *arr = list->data;
    if(list->destroy != NULL){
        int i = 0;
        for(;i < list->size; i++, arr += list->eSize){
            list->destroy(arr);
        }
    }

    free(list->data);
    free(list);
}