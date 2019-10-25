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

char transformationType = TRANS_NULL;
char referenceSystem = SYS_REF_LOCAL;
char applyTransTo = OBJECT_TRANS;
char printed = 0;

void imprimir_configuracion() {
    if(printed == 1)
    {
        int i;
        for(i = 0; i <= 4; i++)
        {
            printf("\033[1A"); // Move up X line
            printf("\33[2K"); //Erase the current line
        }
    }
    printf("--------------------------------------\n");
    if(referenceSystem == SYS_REF_LOCAL)
    {
        printf("Sistema de Referencia: Local\n");
    } else {
        printf("Sistema de Referencia: Global\n");
    }
    if(transformationType == TRANS_TRASLATE)
    {
        printf("Tipo Transformacion: Translacion\n");;
    } else if(transformationType == TRANS_ROTATE) {
        printf("Tipo Transformacion: Rotacion\n");
    } else if(transformationType == TRANS_SCALE) {
        printf("Tipo Transformacion: Escalado\n");
    } else {
        printf("Tipo Transformacion: Ninguno\n");
    }
    if(applyTransTo == OBJECT_TRANS)
    {
        printf("Aplicar a: Objeto\n");
    } else if(applyTransTo == CAMERA_TRANS) {
        printf("Aplicar a: Camara\n");
    } else {
        printf("Aplicar a: Luz\n");
    }
    printf("--------------------------------------\n");
    printed = 1;
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
    free(optr->matrix_list);
    free(optr->face_table);
    free(optr->vertex_table);
    free(optr);
}

void enlazar_matriz(object3d *optr)
{
    modelviewElem *newModelView = malloc(sizeof(modelviewElem));
    glGetFloatv(GL_MODELVIEW_MATRIX, newModelView->modelview);
    newModelView->next = optr->matrix_list;
    optr->matrix_list = newModelView;
}

void aplicateTransformations(int key)
{
    float xTrans = 0;
    float yTrans = 0;
    float zTrans = 0;

    float xRot = 0;
    float yRot = 0;
    float zRot = 0;

    float xScal = 1;
    float yScal = 1;
    float zScal = 1;

    if(key == GLUT_KEY_UP) //Trasladar +Y; Escalar + Y; Rotar +X
    {
        yTrans = KG_STEP_MOVE;
        xRot = 1;
        yScal = 2;
    } else if(key == GLUT_KEY_DOWN) { //Trasladar -Y; Escalar - Y; Rotar -X
        yTrans = -1*KG_STEP_MOVE;
        xRot = -1;
        yScal = 0.5;
    } else if(key == GLUT_KEY_RIGHT) { //Trasladar +X; Escalar + X; Rotar +Y
        xTrans = KG_STEP_MOVE;
        yRot = 1;
        xScal = 2;
    } else if(key == GLUT_KEY_LEFT) { //Trasladar -X; Escalar - X; Rotar -Y
        xTrans = -1*KG_STEP_MOVE;
        yRot = -1;
        xScal = 0.5;
    } else if(key == GLUT_KEY_PAGE_UP) { //Trasladar +Z; Escalar + Z; Rotar +Z
        zTrans = KG_STEP_MOVE;
        zRot = 1;
        zScal = 2;
    } else if(key == GLUT_KEY_PAGE_DOWN) { //Trasladar -Z; Escalar - Z; Rotar -Z
        zTrans = -1*KG_STEP_MOVE;
        zRot = -1;
        zScal = 0.5;
    } else if(key == '+') {
        xScal = 2;
        yScal = 2;
        zScal = 2;
    } else if(key == '-') {
        xScal = 0.5;
        yScal = 0.5;
        zScal = 0.5;
    }
    if(transformationType == TRANS_TRASLATE)
    {
        glTranslatef(xTrans, yTrans, zTrans); 
    } else if(transformationType == TRANS_ROTATE) {
        glRotatef(KG_STEP_ROTATE, xRot, yRot, zRot); 
    } else if(transformationType == TRANS_SCALE) {
        glScalef(xScal, yScal, zScal);
    }
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
            printed = 0;
            imprimir_configuracion();
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
        } else{
            glMatrixMode(GL_MODELVIEW);
            if(referenceSystem == SYS_REF_LOCAL)
            {
                glLoadMatrixf(_selected_object->matrix_list->modelview);
                aplicateTransformations(key);
            } else {
                glLoadIdentity();
                aplicateTransformations(key);
                glMultMatrixf(_selected_object->matrix_list->modelview);
            }
            enlazar_matriz(_selected_object);
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
        } else {
            glMatrixMode(GL_MODELVIEW);
            if(referenceSystem == SYS_REF_LOCAL)
            {
                glLoadMatrixf(_selected_object->matrix_list->modelview);
                aplicateTransformations(key);
            } else {
                glLoadIdentity();
                aplicateTransformations(key);
                glMultMatrixf(_selected_object->matrix_list->modelview);
            }
            enlazar_matriz(_selected_object);
        }
        break;

    case 'm':
    case 'M':
        transformationType = TRANS_TRASLATE;
        imprimir_configuracion();
        break;

    case 'b':
    case 'B':
        transformationType = TRANS_ROTATE;
        imprimir_configuracion();
        break;

    case 't':
    case 'T':
        transformationType = TRANS_SCALE;
        imprimir_configuracion();
        break;

    case 'g':
    case 'G':
        referenceSystem = SYS_REF_GLOBAL;
        imprimir_configuracion();
        break;

    case 'l':
    case 'L':
        referenceSystem = SYS_REF_LOCAL;
        imprimir_configuracion();
        break;

    case 'o':
    case 'O':
        applyTransTo = OBJECT_TRANS;
        imprimir_configuracion();
        break;

    case 'k':
    case 'K':
        applyTransTo = CAMERA_TRANS;
        imprimir_configuracion();
        break;

    case 'a':
    case 'A':
        applyTransTo = LIGHT_TRANS;
        imprimir_configuracion();
        break;

    case '?':
        print_help();
        break;

    case 27: /* <ESC> */
        //Funcion de borrado de memoria
        exit(0);
        break;

    case 'z':
    case 'Z':
        if(_selected_object != MURPHY)
        {
            if(_selected_object->matrix_list->next != MURPHY)
            {
                modelviewElem *aux =_selected_object->matrix_list;
                _selected_object->matrix_list = aux->next;

                free(aux);
            }
        }
        break;

    default:
        /*In the default case we just print the code of the key. This is usefull to define new cases*/
        printf("%d %c\n", key, key);
        printed = 0;
    }
    /*In case we have do any modification affecting the displaying of the object, we redraw them*/
    glutPostRedisplay();
}

void specialKeyboard(int key, int x, int y)
{
    glMatrixMode(GL_MODELVIEW);
    if(referenceSystem == SYS_REF_LOCAL)
    {
        glLoadMatrixf(_selected_object->matrix_list->modelview);
        aplicateTransformations(key);
    } else {
        glLoadIdentity();
        aplicateTransformations(key);
        glMultMatrixf(_selected_object->matrix_list->modelview);
    }
    enlazar_matriz(_selected_object);

    glutPostRedisplay();
}

