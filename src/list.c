/**
 * List file holding the functionalities necessary for a dynamic list (not linked list).
 * 
 * Author: Fabio Hux
 * 
 * Date Created: 5/24/2020
 * 
 * Date Last Edited: 5/24/2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

/**
 * Function to append an element to the end of the list.
 * 
 * Function will add the element to the end of the list, adjusting the size of the list if necessary.
 * 
 * Will return a 0 or 1 on failure or success, respectively. Function will exit the program in the event that it cannot reallocate space for the list.
 */
char append(List *list, void *value){
    insertionResize(list);

    return set(list, list->size++, value);
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
        insertionResize(list);

        int i = list->size++;
        while(i > index){
            set(list, i, list->data + (i - 1));
            --i;
        }

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
 * Function will compare the size and the length and in the even that both values are the same, it will realloc the list to twice the length.
 * 
 * NOTE: Should not be used by outside user, rather a helper function for append(), insert(), and insertSorted().
 */
void insertionResize(List *list){
    if(list->size == list->len){
        list->data = realloc(list->data, sizeType(list->dType) * (list->len * 2));
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

    void *value = calloc(1,sizeType(list->dType));

    if(value == NULL){
        printf("An error occurred with malloc in remove. Exiting.\n");
        exit(0);
    }

    copyVal(list->data + index, value, list->dType);

    int i = index;
    while(i < list->size - 1){
        set(list, i, list->data + (i + 1));
        ++i;
    }

    list->size--;
    return value;
}

/**
 * Function to remove an element from the list at a given index (if valid), returning the removed element.
 * 
 * Function is identical to removeRet() (see removeRet()) with the exception that the return value will not be the element, rather a 0 or 1 indicating the failure or success, respectively.
 * 
 * NOTE: In the case that the list is of strings or lists, it will be freed by the function.
 */
int removeNoRet(List *list, int index){
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
}

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

    return list->data + index;
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

    switch(list->dType){
        case 0:
            ((int *) list->data)[index] = *((int *) value);
            break;
        case 1:
            ((long *) list->data)[index] = *((long *) value);
            break;
        case 2:
            ((float *) list->data)[index] = *((float *) value);
            break;
        case 3:
            ((double *) list->data)[index] = *((double *) value);
            break;
        case 4:
            ((char *) list->data)[index] = *((char *) value);
            break;
        case 5:
            if(*((char **) list->data + index) != NULL)
                free(*((char **) list->data + index));
            ((char **) list->data)[index] = ((char *) value);
            break;
        case 6:
            if(*((List **) list->data + index) != NULL)
                deleteDataList(*((List **) list->data + index));
            ((List **) list->data)[index] = ((List *) value);
            break;
        default:
            return 0;
    }
    
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
    int index = closestIndexOf(list, value);

    if(index >= list->size || cmpVal(list->data + index, value, list->dType)){
        index = -1;
    }

    return index;
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
    int low = 0, high = list->size - 1, mid, diff;

    while(1){
        if(cmpVal(list->data + low, value, list->dType) > 0){
            return low;
        }else if(cmpVal(list->data + high, value, list->dType) < 0){
            return high + 1;
        }else{
            mid = (high + low) >> 1;
            diff = cmpVal(list->data + mid, value, list->dType);
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
int cmpVal(void *a, void *b, int flag){
    int diff;

    switch(flag){
        case 0:
            diff = *((int *)a) - *((int *)b);
            break;
        case 1:
            diff = (int) (*((long *)a) - *((long *)b));
            break;
        case 2:
            {float diffVal = *((float *) a) - *((float *) b);
            if(diffVal > -0.0000001 && diffVal < 0.0000001){
                diff = 0;
            }else if (diffVal <= -0.0000001){
                diff = ((int) diffVal) - 1;
            }else{
                diff = ((int) diffVal) + 1;
            }}
            break;
        case 3:
            {double diffVal = *((double *) a) - *((double *) b);
            if(diffVal > -0.0000001 && diffVal < 0.0000001){
                diff = 0;
            }else if (diffVal <= -0.0000001){
                diff = ((int) diffVal) - 1;
            }else{
                diff = ((int) diffVal) + 1;
            }}
            break;
        case 4:
            diff = (int) (*((char *) a) - *((char *) b));
            break;
        case 5:
            diff = strcmp(*((char **) a), *((char **) b));
            break;
        case 6:
            if(*((List **) a) == *((List **) b)){
                diff = 0;
            }else if(*((List **) a) < *((List **) b)){
                diff = -1;
            }else{
                diff = 1;
            }
        default:
            return 0;
    }
    
    return diff;
}

/**
 * Function to create a list of a given length and certain type.
 * 
 * Function will create a list to return to the caller that is built to the specific length and type which the user has requested for.
 * NULL will be returned in the case that: A)The len given is <= 0. B)The type given does not follow one of the given types in the list ["int", "long", "float", "double", "char", "string", "list"].
 * 
 * NOTE: Function will cause an exit in the case mallocing the list or the data results in a NULL pointer.
 */
List *createDataList(int len, char *type){
    if(len <= 0) return NULL;

    char flag;

    char i;

    for(i = 0; i < 7; ++i){
        if(!strcmp(type, *(types + i))){
            flag = i;
            break;
        }
    }

    if(i == 7){
        return NULL;
    }

    List *list = (List *) calloc(1,sizeof(List));

    if(list == NULL){
        printf("An error occurred with malloc in createDataList on creating the list. Exiting.\n");
        exit(0);
    }

    switch(flag){
        case 0:
            list->data = calloc(1,sizeof(int) * len);
        case 1:
            list->data = calloc(1,sizeof(long) * len);
        case 2:
            list->data = calloc(1,sizeof(float) * len);
        case 3:
            list->data = calloc(1,sizeof(double) * len);
        case 4:
            list->data = calloc(1,sizeof(char) * len);
        case 5:
            list->data = calloc(1,sizeof(char *) * len);
        case 6:
            list->data = calloc(1,sizeof(List *) * len);
    }

    if(list->data == NULL){
        printf("An error occurred with malloc in createDataList on setting the data list. Exiting.\n");
        exit(0);
    }

    list->size = 0;
    list->len = len;
    list->dType = flag;

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

    switch(list->dType){
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
            free(list->data);
            break;
        case 5:
            while(list->size > 0){
                char **val = ((char **) removeRet(list, 0));
                if(val != NULL){
                    free(*val);
                    free(val);
                }
            }
            free(list->data);
            break;
        case 6:
            while(list->size > 0){
                List **l = ((List **) removeRet(list, 0));
                if(l != NULL){
                    deleteDataList(*l);
                    free(l);
                }
            }
            free(list->data);
            break;
    }

    free(list);
}

/**
 * 
 */
void copyVal(void *src, void *trg, int flag){
    if(src == NULL || trg == NULL) return;

    switch(flag){
        case 0:
            *((int *) trg) = *((int *) src);
        case 1:
            *((long *) trg) = *((long *) src);
        case 2:
            *((float *) trg) = *((float *) src);
        case 3:
            *((double *) trg) = *((double *) src);
        case 4:
            *((char *) trg) = *((char *) src);
        case 5:
            *((char **) trg) = *((char **) src);
        case 6:
            *((List **) trg) = *((List **) src);
    }
}

size_t sizeType(int flag){
    switch(flag){
        case 0:
            return sizeof(int);
        case 1:
            return sizeof(long);
        case 2:
            return sizeof(float);
        case 3:
            return sizeof(double);
        case 4:
            return sizeof(char);
        case 5:
            return sizeof(char *);
        case 6:
            return sizeof(List *);
    }
}