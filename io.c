#include "definitions.h"
#include "load_obj.h"
#include <string.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>

extern object3d * _first_object;
extern object3d * _selected_object;

extern GLdouble _ortho_x_min,_ortho_x_max;
extern GLdouble _ortho_y_min,_ortho_y_max;
extern GLdouble _ortho_z_min,_ortho_z_max;

extern GLfloat indentity[16];
extern GLfloat translation[16];
extern GLfloat rotation[16];
extern GLfloat scalation[16];

char trasnformationMode[] = {0, 0, 0};
char trasnformationActive = 0;
char referenceSystem = 0;
char transformationType = 1;

/*
TrasformationMOde --> Modo de transformación:

1 byte / 3 primero bit
        T   R   E
00000   0   0   0

ReferenceSystem --> Sistema de referenica:

1 byte:
    0 : Sin refferencia //Error
    1 : Mundo
    2 : Local del Objeto

TransformationType --> Elemento a transformar:

    1 byte:
        1 : Objeto
        2 : Camara
        3 : Luz 
*/

void imprimir_estado_transformaciones() {
    char str[70];
    if(trasnformationMode[0] == 1)
    {
        strcpy(str, "Translacion: Activa\n");
    } else {
        strcpy(str, "Translacion: Desactiva\n");
    }

    if(trasnformationMode[1] == 1)
    {
        strcat(str, "Rotacion: Activa\n");
    } else {
        strcat(str, "Rotacion: Desactiva\n");
    }

    if(trasnformationMode[2] == 1)
    {
        strcat(str, "Escalado: Activa\n");
    } else {
        strcat(str, "Escalado: Desactiva\n");
    }
    printf("%s\n", str);
}

/**
 * @brief This function just prints information about the use
 * of the keys
 */
void print_help(){
    printf("KbG Irakasgaiaren Praktika. Programa honek 3D objektuak \n");
    printf("aldatzen eta bistaratzen ditu.  \n\n");
    printf("\n\n");
    printf("FUNTZIO NAGUSIAK \n");
    printf("<?>\t\t Laguntza hau bistaratu \n");
    printf("<ESC>\t\t Programatik irten \n");
    printf("<F>\t\t Objektua bat kargatu\n");
    printf("<TAB>\t\t Kargaturiko objektuen artean bat hautatu\n");
    printf("<DEL>\t\t Hautatutako objektua ezabatu\n");
    printf("<CTRL + ->\t Bistaratze-eremua handitu\n");
    printf("<CTRL + +>\t Bistaratze-eremua txikitu\n");
    printf("\n\n");
}

void liberar(object3d *optr)
{
    int i;
    for(i=0; i<optr->num_faces; i++) 
    {
        free(optr->face_table[i].vertex_table);
    }
    free(optr->face_table);
    free(optr->vertex_table);
    free(optr);
}

void enlazar_matriz(object3d *optr)
{
    GLfloat model[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, model);
    matrix_list *matptr = malloc(sizeof(matrix_list));
    int i;
    for(i=0; i<16; i++)
    {
        matptr->modelview[i] = model[i]; 
    }
    matptr->next = optr->matrix_list;
    optr->matrix_list = matptr;
}

//glGetFloat(GL_MODEL_VIEW_MATRIX, *puntero_en_donde_guardar)

/**
 * @brief Callback function to control the basic keys
 * @param key Key that has been pressed
 * @param x X coordinate of the mouse pointer when the key was pressed
 * @param y Y coordinate of the mouse pointer when the key was pressed
 */
void keyboard(unsigned char key, int x, int y) {

    char* fname = malloc(sizeof (char)*128); /* Note that scanf adds a null character at the end of the vector*/
    int read = 0;
    object3d *auxiliar_object = 0;
    GLdouble wd,he,midx,midy;

    switch (key) {
    case 'f':
    case 'F':
        /*Ask for file*/
        printf("%s", KG_MSSG_SELECT_FILE);
        scanf("%s", fname);
        /*Allocate memory for the structure and read the file*/
        auxiliar_object = (object3d *) malloc(sizeof (object3d));
        read = read_wavefront(fname, auxiliar_object);
        switch (read) {
        /*Errors in the reading*/
        case 1:
            printf("%s: %s\n", fname, KG_MSSG_FILENOTFOUND);
            break;
        case 2:
            printf("%s: %s\n", fname, KG_MSSG_INVALIDFILE);
            break;
        case 3:
            printf("%s: %s\n", fname, KG_MSSG_EMPTYFILE);
            break;
        /*Read OK*/
        case 0:
            /*Insert the new object in the list*/
            auxiliar_object->next = _first_object;
            _first_object = auxiliar_object;
            _selected_object = _first_object;
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            enlazar_matriz(_selected_object);
            printf("%s\n",KG_MSSG_FILEREAD);
            break;
        }
        break;

    case 9: /* <TAB> */
        _selected_object = _selected_object->next;
        /*The selection is circular, thus if we move out of the list we go back to the first element*/
        if (_selected_object == 0) _selected_object = _first_object;
        break;

    case 127: /* <SUPR> */
        if(_first_object == 0)
        {
            printf("No cuela\n");
            break;
        }
        /*Erasing an object depends on whether it is the first one or not*/
        if (_selected_object == _first_object)
        {
            /*To remove the first object we just set the first as the current's next*/
            _first_object = _first_object->next;
            /*Once updated the pointer to the first object it is save to free the memory*/
            liberar(_selected_object);
            /*Finally, set the selected to the new first one*/
            _selected_object = _first_object;
        } else {
            /*In this case we need to get the previous element to the one we want to erase*/
            auxiliar_object = _first_object;
            while (auxiliar_object->next != _selected_object)
                auxiliar_object = auxiliar_object->next;
            /*Now we bypass the element to erase*/
            auxiliar_object->next = _selected_object->next;
            /*free the memory*/
            liberar(_selected_object);
            /*and update the selection*/
            _selected_object = auxiliar_object;
        }
        break;

    case '-':
        if (glutGetModifiers() == GLUT_ACTIVE_CTRL)
        {
            /*Increase the projection plane; compute the new dimensions*/
            wd=(_ortho_x_max-_ortho_x_min)/KG_STEP_ZOOM;
            he=(_ortho_y_max-_ortho_y_min)/KG_STEP_ZOOM;
            /*In order to avoid moving the center of the plane, we get its coordinates*/
            midx = (_ortho_x_max+_ortho_x_min)/2;
            midy = (_ortho_y_max+_ortho_y_min)/2;
            /*The the new limits are set, keeping the center of the plane*/
            _ortho_x_max = midx + wd/2;
            _ortho_x_min = midx - wd/2;
            _ortho_y_max = midy + he/2;
            _ortho_y_min = midy - he/2;
        }
        break;

    case '+':
        //INPLEMENTA EZAZU CTRL + + KONBINAZIOAREN FUNTZIOANLITATEA
        if (glutGetModifiers() == GLUT_ACTIVE_CTRL)
        {
            /*Increase the projection plane; compute the new dimensions*/
            wd=(_ortho_x_max-_ortho_x_min)*KG_STEP_ZOOM;
            he=(_ortho_y_max-_ortho_y_min)*KG_STEP_ZOOM;
            /*In order to avoid moving the center of the plane, we get its coordinates*/
            midx = (_ortho_x_max+_ortho_x_min)/2;
            midy = (_ortho_y_max+_ortho_y_min)/2;
            /*The the new limits are set, keeping the center of the plane*/
            _ortho_x_max = midx + wd/2;
            _ortho_x_min = midx - wd/2;
            _ortho_y_max = midy + he/2;
            _ortho_y_min = midy - he/2;
        }
        break;

    case 'm':
    case 'M':
        if(trasnformationMode[0] == 1)
        {
            trasnformationMode[0] = 0;
        } else {
            trasnformationMode[0] = 1;
        }
        imprimir_estado_transformaciones();
        break;

    case 'b':
    case 'B':
        if(trasnformationMode[1] == 1)
        {
            trasnformationMode[1] = 0;
        } else {
            trasnformationMode[1] = 1;
        }
        imprimir_estado_transformaciones();
        break;

    case 't':
    case 'T':
        if(trasnformationMode[2] == 1)
        {
            trasnformationMode[2] = 0;
        } else {
            trasnformationMode[2] = 1;
        }
        imprimir_estado_transformaciones();
        break;

    case 'g':
    case 'G':
        referenceSystem = 1;
        printf("Sistema de referencia: Mundo\n\n");
        break;

    case 'l':
    case 'L':
        referenceSystem = 2;
        printf("Sistema de referencia: Local\n\n");
        break;

    case 'o':
    case 'O':
        transformationType = 1;
        printf("Aplicar transformacion a: Objeto\n\n");
        break;

    case 'k':
    case 'K':
        transformationType = 2;
        printf("Aplicar transformacion a: Camara\n\n");
        break;

    case 'a':
    case 'A':
        transformationType = 3;
        printf("Aplicar transformacion a: Luz\n\n");
        break;

    case 72:
        printf("Hola\n");
        break;

    case '?':
        print_help();
        break;

    case 27: /* <ESC> */
        //Funcion de borrado de memoria
        exit(0);
        break;

    default:
        /*In the default case we just print the code of the key. This is usefull to define new cases*/
        printf("%d %c\n", key, key);
    }
    /*In case we have do any modification affecting the displaying of the object, we redraw them*/
    glutPostRedisplay();
}

