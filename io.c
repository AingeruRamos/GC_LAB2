#include "definitions.h"
#include "load_obj.h"
#include <string.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <math.h>

extern object3d * _first_object;
extern object3d * _selected_object;
extern camera *_cameras;
extern camera *_selected_camera;
extern camera *objectCamera;

extern GLdouble _ortho_x_min,_ortho_x_max;
extern GLdouble _ortho_y_min,_ortho_y_max;
extern GLdouble _ortho_z_min,_ortho_z_max;

extern camera *objectCamera;
extern int projectionType;

char transformationType = TRANS_NULL;
char referenceSystem = SYS_REF_LOCAL;
char applyTransTo = OBJECT_TRANS;
char printed = 0;

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

void liberar(object3d *optr)
{
    int i;
    for(i=0; i<optr->num_faces; i++) 
    {
        free(optr->face_table[i].vertex_table);
    }
    free(optr->modelview_list);
    free(optr->face_table);
    free(optr->vertex_table);
    free(optr);
}

void enlazar_matriz_objeto(object3d *optr)
{
    modelview *newModelView = malloc(sizeof(modelview));
    glGetFloatv(GL_MODELVIEW_MATRIX, newModelView->value);
    newModelView->next = optr->modelview_list;
    optr->modelview_list = newModelView;
}

void enlazar_matriz_camara(camera *cam)
{
    modelview *newModelView = malloc(sizeof(modelview));
    glGetFloatv(GL_MODELVIEW_MATRIX, newModelView->value);
    newModelView->next = cam->camera_matrix_list;
    cam->camera_matrix_list = newModelView;
}

void* getSelectedObjectPosition(GLfloat *resultMatrix) 
{
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(_selected_object->modelview_list->value);
    GLfloat m[16];
    for(int i=0; i<4; i++) {
        for(int j=0; j< 4; j++) {
            if(i == 0) {
                m[4*i + j] = 1;
            } else {
                m[4*i+j] = 0;
            }
        }
    }
    glMultMatrixf(m);
    glGetFloatv(GL_MODELVIEW_MATRIX, m);
    for(int i=0; i<4; i++)
    {
        for(int j=0; j<4; j++)
        {
            resultMatrix[4*i+j] = m[4*i+j];
        }
    }
    resultMatrix[0] -= 1;
    resultMatrix[1] -= 1;
    resultMatrix[2] -= 1;
}

camera* createNewCamera(float xe, float ye, float ze, float xat, float yat, float zat)
{
    camera *newCamera = malloc(sizeof(camera));
    newCamera->xPos = xe;
    newCamera->yPos = ye;
    newCamera->zPos = ze;
    newCamera->x = 0.1;
    newCamera->y = 0.1;
    newCamera->near = 0.1;
    newCamera->far = 1000; 
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(xe, ye, ze, xat, yat, zat, 0.0, 1.0, 0.0);
    modelview *newModelView = malloc(sizeof(modelview));
    glGetFloatv(GL_MODELVIEW_MATRIX, newModelView->value);
    newCamera->camera_matrix_list = newModelView;
    return newCamera;
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

    float xCamera = 0;
    float yCamera = 0;
    float dNearFar = 0;

    if(key == GLUT_KEY_UP) //Trasladar +Y; Escalar + Y; Rotar +X
    {
        yTrans = KG_STEP_MOVE;
        xRot = 1;
        yScal = 2;
        yCamera = 0.1;
    } else if(key == GLUT_KEY_DOWN) { //Trasladar -Y; Escalar - Y; Rotar -X
        yTrans = -1*KG_STEP_MOVE;
        xRot = -1;
        yScal = 0.5;
        yCamera = -0.1;
    } else if(key == GLUT_KEY_RIGHT) { //Trasladar +X; Escalar + X; Rotar +Y
        xTrans = KG_STEP_MOVE;
        yRot = 1;
        xScal = 2;
        xCamera = 0.1;
    } else if(key == GLUT_KEY_LEFT) { //Trasladar -X; Escalar - X; Rotar -Y
        xTrans = -1*KG_STEP_MOVE;
        yRot = -1;
        xScal = 0.5;
        xCamera = -0.1;
    } else if(key == GLUT_KEY_PAGE_UP) { //Trasladar +Z; Escalar + Z; Rotar +Z
        zTrans = KG_STEP_MOVE;
        zRot = 1;
        zScal = 2;
        dNearFar = 0.1;
    } else if(key == GLUT_KEY_PAGE_DOWN) { //Trasladar -Z; Escalar - Z; Rotar -Z
        zTrans = -1*KG_STEP_MOVE;
        zRot = -1;
        zScal = 0.5;
        dNearFar = -0.1;
    } else if(key == '+') {
        xScal = 2;
        yScal = 2;
        zScal = 2;
        xCamera = -0.1;
        yCamera = -0.1;
    } else if(key == '-') {
        xScal = 0.5;
        yScal = 0.5;
        zScal = 0.5;
        xCamera = 0.1;
        yCamera = 0.1;
    }

    if(applyTransTo == OBJECT_TRANS) {
        if(transformationType == TRANS_TRASLATE)
        {
            glTranslatef(xTrans, yTrans, zTrans); 
        } else if(transformationType == TRANS_ROTATE) {
            glRotatef(KG_STEP_ROTATE, xRot, yRot, zRot); 
        } else if(transformationType == TRANS_SCALE) {
            glScalef(xScal, yScal, zScal);
        }
    } else if(applyTransTo == CAMERA_TRANS) {
        if(referenceSystem == SYS_REF_LOCAL)
        {
            if(transformationType == TRANS_TRASLATE)
            {
                glTranslatef(-1*xTrans, -1*yTrans, zTrans);
            } else if(transformationType == TRANS_ROTATE) {
                glRotatef(KG_STEP_ROTATE, -1*xRot, yRot, zRot);
            } else if(transformationType == TRANS_SCALE) {
                _selected_camera->x += xCamera;
                _selected_camera->y += yCamera;
                _selected_camera->near += dNearFar;
                _selected_camera->far += dNearFar;
            }
        } else {
            glRotatef(KG_STEP_ROTATE, -1*xRot, -1*yRot, zRot);
        }
    }
}

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
            enlazar_matriz_objeto(_selected_object);
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
                glLoadMatrixf(_selected_object->modelview_list->value);
                aplicateTransformations(key);
            } else {
                glLoadIdentity();
                aplicateTransformations(key);
                glMultMatrixf(_selected_object->modelview_list->value);
            }
            enlazar_matriz_objeto(_selected_object);
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
                glLoadMatrixf(_selected_object->modelview_list->value);
                aplicateTransformations(key);
            } else {
                glLoadIdentity();
                aplicateTransformations(key);
                glMultMatrixf(_selected_object->modelview_list->value);
            }
            enlazar_matriz_objeto(_selected_object);
        }
        break;

    case 'n':
    case 'N':
        printf("Posicion de la camara:\n");
        float xe;
        printf("\tx: "); scanf("%f", &xe);
        float ye;
        printf("\ty: "); scanf("%f", &ye);
        float ze;
        printf("\tz: "); scanf("%f", &ze);
        printf("Posicion de la vista:\n");
        float xat;
        printf("\tx: "); scanf("%f", &xat);
        float yat;
        printf("\ty: "); scanf("%f", &yat);
        float zat;
        printf("\tz: "); scanf("%f", &zat);

        camera *newCamera = createNewCamera(xe, ye, ze, xat, yat, zat);
        newCamera->next = _cameras;
        _cameras = newCamera;
        _selected_camera = _cameras;

        printf("La camara se ha creado con exito\n");
        break;
    
    case 'c':
        if(objectCamera == 0)
        {
            if(_selected_camera->next == 0)
            {
                _selected_camera = _cameras;
            } else {
                _selected_camera = _selected_camera->next;
            }
        } else {
            _selected_camera = _cameras;
        }
        break;
    
    case 'C':
        break;
        
    case 'p':
        if(projectionType == 0)
        {
            projectionType = 1;
        } else {
            projectionType = 0;
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
        if(applyTransTo == CAMERA_TRANS)
        {
            GLfloat pos[16];
            getSelectedObjectPosition(pos);
            camera *newCamera = createNewCamera(pos[0], pos[1], (pos[2]+10), pos[0], pos[1], pos[2]);
            newCamera->next = _cameras;
            _cameras = newCamera;
            _selected_camera = _cameras;

        }
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
            if(_selected_object->modelview_list->next != MURPHY)
            {
                modelview *aux =_selected_object->modelview_list;
                _selected_object->modelview_list = aux->next;
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
    if(applyTransTo == OBJECT_TRANS)
    {
        if(referenceSystem == SYS_REF_LOCAL)
        {
            glLoadMatrixf(_selected_object->modelview_list->value);
            aplicateTransformations(key);
        } else {
            glLoadIdentity();
            aplicateTransformations(key);
            glMultMatrixf(_selected_object->modelview_list->value);
        }
        enlazar_matriz_objeto(_selected_object);
    } else if(applyTransTo == CAMERA_TRANS) {
        if(referenceSystem == SYS_REF_LOCAL)
        {
            glLoadIdentity();
            aplicateTransformations(key);
            glMultMatrixf(_selected_camera->camera_matrix_list->value);
        } /*else {
            
            glLoadIdentity();
            GLfloat at[16];
            getSelectedObjectPosition(at);
            float dx = _selected_camera->xPos - at[0];
            dx *= dx;
            float dy = _selected_camera->yPos - at[1];
            dy *= dy;
            float dz = _selected_camera->zPos - at[2];
            dz *= dz;
            float d = sqrt(dx + dy + dz);
            glTranslatef(0, 0, -1*d);
            aplicateTransformations(key);
            glTranslatef(0, 0, d);
            glMultMatrixf(_selected_camera->camera_matrix_list->value);
        }
        */
        enlazar_matriz_camara(_selected_camera);
    }
    glutPostRedisplay();
}

