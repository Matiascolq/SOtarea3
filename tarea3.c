#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Inode {
    char name[100];
    int size;
    int permissions;
    int id;
    struct Inode *parent;
    struct Inode **children; // Array de punteros a i-nodos hijos.
    int childCount;          // Contador de hijos.
} Inode;

Inode *currentDirectory; // Referencia global al directorio actual.


Inode *createInode(char *name, int size, int permissions, int id, Inode *parent) {
    Inode *newInode = (Inode *)malloc(sizeof(Inode));
    strcpy(newInode->name, name);
    newInode->size = size;
    newInode->permissions = permissions;
    newInode->id = id;
    newInode->parent = parent;
    newInode->children = NULL;
    newInode->childCount = 0;
    return newInode;
}

void addChild(Inode *parent, Inode *child) {
    parent->childCount++;
    parent->children = realloc(parent->children, parent->childCount * sizeof(Inode *));
    parent->children[parent->childCount - 1] = child;
}

void printInode(Inode *inode) {
    printf("Nombre: %s\n", inode->name);
    printf("Tamaño: %d\n", inode->size);
    printf("Permisos: %d\n", inode->permissions);
    printf("ID: %d\n", inode->id);
    if (inode->parent) {
        printf("Padre: %s\n", inode->parent->name);
    } else {
        printf("Padre: Ninguno\n");
    }
    printf("Hijos: ");
    if (inode->childCount == 0) {
        printf("Ninguno\n");
    } else {
        for (int i = 0; i < inode->childCount; i++) {
            printf("%s ", inode->children[i]->name);
        }
        printf("\n");
    }
}

void printInodeTree(Inode *inode) {
    printInode(inode);
    for (int i = 0; i < inode->childCount; i++) {
        printInodeTree(inode->children[i]);
    }
}

void deleteInode(Inode *inode) {
    for (int i = 0; i < inode->childCount; i++) {
        deleteInode(inode->children[i]);
    }
    free(inode->children);
    free(inode);
}

//Funcion para cambiar de directorio
void cd(Inode **currentDir, char *name) {
    for (int i = 0; i < (*currentDir)->childCount; i++) {
        if (strcmp((*currentDir)->children[i]->name, name) == 0) {
            *currentDir = (*currentDir)->children[i];
            return;
        }
    }
    printf("No existe el directorio: %s\n", name);
}

//Funcion para renombrar archivos o directorios
void renameInode(Inode *inode, char *name, char *newName) {
    for (int i = 0; i < inode->childCount; i++) {
        if (strcmp(inode->children[i]->name, name) == 0) {
            strcpy(inode->children[i]->name, newName);
            return;
        }
    }
    printf("No existe el directorio\n");
}

//Funcion para cambiar permisos de archivos o directorios
void chmod(Inode *inode, char *name, int permissions) {
    for (int i = 0; i < inode->childCount; i++) {
        if (strcmp(inode->children[i]->name, name) == 0) {
            inode->children[i]->permissions = permissions;
            return;
        }
    }
    printf("No existe el directorio\n");
}

//Funcion para obtener metadata de archivos o directorios
void stat(Inode *inode, char *name) {
    for (int i = 0; i < inode->childCount; i++) {
        if (strcmp(inode->children[i]->name, name) == 0) {
            printInode(inode->children[i]);
            return;
        }
    }
    printf("No existe el directorio\n");
}

//Funcion para listar archivos y directorios
void ls(Inode *inode) {
    for (int i = 0; i < inode->childCount; i++) {
        printf("%s\n", inode->children[i]->name);
    }
}

//Funcion para mover archivos y directorios
void mv(Inode *inode, char *name, Inode *newParent) {
    for (int i = 0; i < inode->childCount; i++) {
        if (strcmp(inode->children[i]->name, name) == 0) {
            Inode *childToMove = inode->children[i];
            // Eliminar de la lista actual
            for (int j = i; j < inode->childCount - 1; j++) {
                inode->children[j] = inode->children[j + 1];
            }
            inode->childCount--;
            inode->children = realloc(inode->children, inode->childCount * sizeof(Inode *));
            // Agregar al nuevo padre
            addChild(newParent, childToMove);
            childToMove->parent = newParent;
            return;
        }
    }
    printf("No existe el directorio: %s\n", name);
}

//Funcion para eliminar archivos y directorios
void rm(Inode *inode, char *name) {
    for (int i = 0; i < inode->childCount; i++) {
        if (strcmp(inode->children[i]->name, name) == 0) {
            deleteInode(inode->children[i]); // Elimina recursivamente
            // Reorganizar la lista de hijos
            for (int j = i; j < inode->childCount - 1; j++) {
                inode->children[j] = inode->children[j + 1];
            }
            inode->childCount--;
            inode->children = realloc(inode->children, inode->childCount * sizeof(Inode *));
            return;
        }
    }
    printf("No existe el directorio: %s\n", name);
}

//Funcion para crear archivos
void touch(Inode *inode, char *name) {
    Inode *newInode = createInode(name, 0, 0, inode->childCount, inode);
    addChild(inode, newInode);
}

//Funcion para crear directorios
void mkdir(Inode *inode, char *name) {
    Inode *newInode = createInode(name, 0, 0, inode->childCount, inode);
    addChild(inode, newInode);
}

//Funcion para guardar y cargar la estructura del arbol en un archivo
void save(Inode *inode, FILE *file) {
    fwrite(inode, sizeof(Inode), 1, file);
    for (int i = 0; i < inode->childCount; i++) {
        save(inode->children[i], file);
    }
}




int main() {
    Inode *root = createInode("root", 0, 0, 0, NULL);
    Inode *home = createInode("home", 0, 0, 1, root);
    Inode *user1 = createInode("user1", 0, 0, 2, home);
    Inode *user2 = createInode("user2", 0, 0, 3, home);
    Inode *user3 = createInode("user3", 0, 0, 4, home);
    addChild(root, home);
    addChild(home, user1);
    addChild(home, user2);
    addChild(home, user3);
    printInodeTree(root);
    deleteInode(root);
    return 0;
}
