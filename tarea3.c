#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Inode {
    char name[100];
    int size;
    int permissions;
    int id;
    struct Inode *parent;
    struct Inode **children;
    int childCount;
} Inode;

Inode *currentDirectory;

Inode *createInode(char *name, int size, int permissions, int id, Inode *parent) {
    Inode *newInode = (Inode *)malloc(sizeof(Inode));
    if (!newInode) {
        perror("Error al asignar memoria para Inode");
        exit(EXIT_FAILURE);
    }
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
    if (!parent->children) {
        perror("Error al reasignar memoria para los hijos del Inode");
        exit(EXIT_FAILURE);
    }
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

void cd(Inode **currentDir, char *name) {
    if (strcmp(name, ".") == 0) {
        // No hacer nada si es el directorio actual
        return;
    }
    if (strcmp(name, "..") == 0) {
        if ((*currentDir)->parent != NULL) {
            *currentDir = (*currentDir)->parent;
        } else {
            printf("Ya estás en el directorio raíz.\n");
        }
    } else {
        for (int i = 0; i < (*currentDir)->childCount; i++) {
            if (strcmp((*currentDir)->children[i]->name, name) == 0 && (*currentDir)->children[i] != *currentDir) {
                *currentDir = (*currentDir)->children[i];
                return;
            }
        }
        printf("No existe el directorio: %s\n", name);
    }
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
    Inode *childToMove = NULL;
    int childIndex = -1;

    for (int i = 0; i < inode->childCount; i++) {
        if (strcmp(inode->children[i]->name, name) == 0) {
            childToMove = inode->children[i];
            childIndex = i;
            break;
        }
    }

    if (childToMove == NULL) {
        printf("No existe el archivo o directorio: %s\n", name);
        return;
    }

    // Eliminar de la lista actual
    for (int i = childIndex; i < inode->childCount - 1; i++) {
        inode->children[i] = inode->children[i + 1];
    }
    inode->childCount--;
    inode->children = realloc(inode->children, inode->childCount * sizeof(Inode *));

    if (inode->children == NULL && inode->childCount > 0) {
        perror("Error al reasignar memoria para los hijos del Inode");
        exit(EXIT_FAILURE);
    }

    // Agregar al nuevo padre
    addChild(newParent, childToMove);
    childToMove->parent = newParent;
}

//Funcion para eliminar archivos y directorios
void rm(Inode *inode, char *name) {
    if (strcmp(inode->name, name) == 0) {
        printf("No se puede eliminar el directorio de trabajo actual.\n");
        return;
    }

    Inode *toDelete = NULL;
    int indexToDelete = -1;
    
    for (int i = 0; i < inode->childCount; i++) {
        if (strcmp(inode->children[i]->name, name) == 0) {
            toDelete = inode->children[i];
            indexToDelete = i;
            break;
        }
    }
    
    if (toDelete == NULL) {
        printf("No existe el directorio: %s\n", name);
        return;
    }
    
    // Verificar que no estamos intentando eliminar el directorio actual o uno por encima
    Inode *tempDir = currentDirectory;
    while (tempDir) {
        if (tempDir == toDelete) {
            printf("Operación no permitida: intento de eliminar el directorio actual o un directorio padre.\n");
            return;
        }
        tempDir = tempDir->parent;
    }
    
    // Eliminar el i-nodo y sus hijos
    deleteInode(toDelete);
    
    // Reorganizar la lista de hijos
    for (int i = indexToDelete; i < inode->childCount - 1; i++) {
        inode->children[i] = inode->children[i + 1];
    }
    inode->childCount--;
    inode->children = realloc(inode->children, inode->childCount * sizeof(Inode *));
}

//Funcion para crear archivos
void touch(Inode *inode, char *name) {
    // Verificar si el archivo ya existe en el directorio actual
    for (int i = 0; i < inode->childCount; i++) {
        if (strcmp(inode->children[i]->name, name) == 0) {
            printf("El archivo '%s' ya existe.\n", name);
            return;
        }
    }
    Inode *newInode = createInode(name, 0, 0666, inode->childCount, inode); // Los archivos por lo general tienen permisos 0666 por defecto
    addChild(inode, newInode);
    printf("Archivo '%s' creado.\n", name); // Mensaje de confirmación
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
    // Inicialización del sistema de archivos
    Inode *root = createInode("root", 0, 0777, 0, NULL);
    currentDirectory = root; // Establece el directorio raíz como directorio actual

    // Crear estructura de directorios y archivos
    mkdir(currentDirectory, "home");
    mkdir(currentDirectory, "etc");
    touch(currentDirectory, "readme.txt");

    // Renombrar archivo
    renameInode(currentDirectory, "readme.txt", "README.md");

    // Cambiar permisos de un archivo
    chmod(currentDirectory, "README.md", 0644);

    // Obtener metadata de un archivo
    stat(currentDirectory, "README.md");

    // Listar archivos y directorios en el directorio actual
    ls(currentDirectory);

    // Moverse al directorio 'home' y crear un archivo
    cd(&currentDirectory, "home");
    touch(currentDirectory, "user_guide.txt");

    // Listar de nuevo para comprobar que 'user_guide.txt' existe en 'home'
    ls(currentDirectory);

    // Intentar mover 'user_guide.txt' desde 'home' a 'root'
    cd(&currentDirectory, ".."); // Asegúrate de estar en el directorio raíz antes de mover el archivo desde 'home'
    Inode *homeDir = NULL;
    for (int i = 0; i < currentDirectory->childCount; i++) {
        if (strcmp(currentDirectory->children[i]->name, "home") == 0) {
            homeDir = currentDirectory->children[i];
            break;
        }
    }
    if (homeDir != NULL) {
        mv(homeDir, "user_guide.txt", currentDirectory); // Asegúrate de pasar 'homeDir' como el directorio donde está 'user_guide.txt'
    } else {
        printf("No se pudo encontrar el directorio 'home'.\n");
    }

    // Listar después de mover para verificar que 'user_guide.txt' se ha movido a 'root'
    ls(currentDirectory);

    // Eliminar el directorio 'etc'
    rm(currentDirectory, "etc");

    // Listar después de eliminar para verificar que 'etc' se ha eliminado
    ls(currentDirectory);

    // Eliminar todos los nodos antes de salir
    deleteInode(root);

    return 0;
}
