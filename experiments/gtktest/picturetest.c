//
// BSD 2-Clause License
//
// Copyright (c) 2024, dettus@dettus.net
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dMagnetic2_errorcodes.h"
#include "dMagnetic2_graphics.h"        // for the xpm function
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>


typedef struct _tHandle
{
	GtkApplication *app;
	GtkWidget *window;
	GtkWidget *picture;
	GdkPixbuf *pixbuf;
	GdkTexture *texture;
	unsigned char drawbuf[4*640*480];

	tdMagnetic2_canvas_small canvas_small;

	void *pGfxHandle;
	void *pTmpBuf;
	int picnum;
	unsigned char pGfxBuf[1<<22];
} tHandle;

static void activate(GtkApplication* app,gpointer user_data)
{
        tHandle* pThis=(tHandle*)user_data;
        pThis->app=app;

        pThis->window=gtk_application_window_new(app);
        gtk_window_set_title(GTK_WINDOW(pThis->window),"hello there!");
        gtk_window_set_default_size(GTK_WINDOW(pThis->window),640,200);

        pThis->picture=gtk_picture_new();
        pThis->pixbuf=gdk_pixbuf_new(GDK_COLORSPACE_RGB,FALSE,8,640,200);
        pThis->texture=gdk_texture_new_for_pixbuf(pThis->pixbuf);
        gtk_picture_set_paintable(GTK_PICTURE(pThis->picture),GDK_PAINTABLE(pThis->texture));


        gtk_window_set_child(GTK_WINDOW(pThis->window),pThis->picture);
        g_signal_connect(G_OBJECT(pThis->window),"close_request",G_CALLBACK(gtk_window_destroy),pThis->window);
        gtk_window_present(GTK_WINDOW(pThis->window));

}


int main(int argc,char** argv)
{
	GtkApplication *app;
	int status;
	void *pHandle;
	int retval;
	int size_handle;
	int size_tmpbuf;
	FILE *f;
	int gfxsize;
	pHandle=malloc(sizeof(tHandle));
	memset(pHandle,0,sizeof(tHandle));

	retval=dMagnetic2_graphics_getsize(&size_handle,&size_tmpbuf);
	pHandle->pGfxHandle=malloc(size_handle);
	pHandle->pTmpBuf=malloc(size_tmpbuf);

	retval=dMagnetic2_graphics_init(pHandle->pGfxHandle,pHandle->pTmpBuf);
	
	f=fopen("guild.gfx","rb");
	gfxsize=fread(pThis->pGfxBuf,sizeof(char),sizeof(pThis->pGfxBuf),f);
	fclose(f);

	dMagnetic2_graphics_set_gfx(pHandle->pGfxHandle,pThis->pGfxBuf,gfxsize);
	pThis->picnum=0;
}


