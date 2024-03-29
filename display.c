#include <stdio.h>
#include "definitions.h"
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>

/** EXTERNAL VARIABLES **/

extern GLdouble _window_ratio;
extern GLdouble _ortho_x_min,_ortho_x_max;
extern GLdouble _ortho_y_min,_ortho_y_max;
extern GLdouble _ortho_z_min,_ortho_z_max;

extern object3d *_first_object;
extern object3d *_selected_object;
extern camera *_cameras;
extern camera *_selected_camera;

extern int projectionType;

/**
 * @brief Function to draw the axes
 */
void draw_axes()
{
    /*Draw X axis*/
    glColor3f(KG_COL_X_AXIS_R,KG_COL_X_AXIS_G,KG_COL_X_AXIS_B);
    glBegin(GL_LINES);
    glVertex3d(KG_MAX_DOUBLE,0,0);
    glVertex3d(-1*KG_MAX_DOUBLE,0,0);
    glEnd();
    /*Draw Y axis*/
    glColor3f(KG_COL_Y_AXIS_R,KG_COL_Y_AXIS_G,KG_COL_Y_AXIS_B);
    glBegin(GL_LINES);
    glVertex3d(0,KG_MAX_DOUBLE,0);
    glVertex3d(0,-1*KG_MAX_DOUBLE,0);
    glEnd();
    /*Draw Z axis*/
    glColor3f(KG_COL_Z_AXIS_R,KG_COL_Z_AXIS_G,KG_COL_Z_AXIS_B);
    glBegin(GL_LINES);
    glVertex3d(0,0,KG_MAX_DOUBLE);
    glVertex3d(0,0,-1*KG_MAX_DOUBLE);
    glEnd();
}


/**
 * @brief Callback reshape function. We just store the new proportions of the window
 * @param width New width of the window
 * @param height New height of the window
 */
void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    /*  Take care, the width and height are integer numbers, but the ratio is a GLdouble so, in order to avoid
     *  rounding the ratio to integer values we need to cast width and height before computing the ratio */
    _window_ratio = (GLdouble) width / (GLdouble) height;
}


/**
 * @brief Callback display function
 */
void display(void) {
    GLint v_index, v, f;
    object3d *aux_obj = _first_object;

    /* Clear the screen */
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    /* Define the projection */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if(projectionType == 0)
    {
        /*When the window is wider than our original projection plane we extend the plane in the X axis*/
        if ((_ortho_x_max - _ortho_x_min) / (_ortho_y_max - _ortho_y_min) < _window_ratio) {
            /* New width*/
            GLdouble wd = (_ortho_y_max - _ortho_y_min) * _window_ratio;
            /* Midpoint in the X axis*/
            GLdouble midpt = (_ortho_x_min + _ortho_x_max) / 2;
            /*Definition of the projection*/
            glOrtho(midpt - (wd / 2), midpt + (wd / 2), _ortho_y_min, _ortho_y_max, _ortho_z_min, _ortho_z_max);
        } else {/* In the opposite situation we extend the Y axis*/
            /* New height*/
            GLdouble he = (_ortho_x_max - _ortho_x_min) / _window_ratio;
            /* Midpoint in the Y axis*/
            GLdouble midpt = (_ortho_y_min + _ortho_y_max) / 2;
            /*Definition of the projection*/
            glOrtho(_ortho_x_min, _ortho_x_max, midpt - (he / 2), midpt + (he / 2), _ortho_z_min, _ortho_z_max);
        }
    } else {
        float x = _selected_camera->x;
        float y = _selected_camera->y;
        float near = _selected_camera->near;
        float far = _selected_camera->far;
        glFrustum(-1*x, x, -1*y, y, near, far);
    }
    /* Now we start drawing the object */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float ambient[] = {0.2125, 0.1275, 0.054, 1.0};
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    float diffuse[] = {0.714, 0.4284, 0.18144};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    float specular[] = {0.393548, 0.271906, 0.166721};
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    float shine = 0.2;
    glMaterialf(GL_FRONT, GL_SHININESS, shine * 128.0);

    float light_position[] = {2, 5, 0};
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glEnable(GL_LIGHT0);

    /*First, we draw the axes*/
    draw_axes();
    int p = 0;
    /*Now each of the objects in the list*/
    while (aux_obj != 0) {
        glLoadMatrixf(_selected_camera->camera_matrix_list->value);
        glMultMatrixf(aux_obj->modelview_list->value);

        /* Select the color, depending on whether the current object is the selected one or not */
        if (aux_obj == _selected_object){
            glColor3f(KG_COL_SELECTED_R,KG_COL_SELECTED_G,KG_COL_SELECTED_B);
        }else{
            glColor3f(KG_COL_NONSELECTED_R,KG_COL_NONSELECTED_G,KG_COL_NONSELECTED_B);
        }

        /* Draw the object; for each face create a new polygon with the corresponding vertices */
        //glLoadIdentity();
        face *faceptr = _selected_object->face_table;
        for (f = 0; f < aux_obj->num_faces; f++) {
            glNormal3fv(faceptr->normalVector);
            glBegin(GL_POLYGON);
            for (v = 0; v < aux_obj->face_table[f].num_vertices; v++) {
                v_index = aux_obj->face_table[f].vertex_table[v];
                glVertex3d(aux_obj->vertex_table[v_index].coord.x,
                        aux_obj->vertex_table[v_index].coord.y,
                        aux_obj->vertex_table[v_index].coord.z);
            }
            glEnd();
            faceptr++;
        }
        aux_obj = aux_obj->next;
    }
    /*Do the actual drawing*/
    glutSwapBuffers();
}
