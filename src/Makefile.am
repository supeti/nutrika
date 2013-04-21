AM_CPPFLAGS = -DREFERENCE_DB=\""$(pkgdatadir)/nutrika.db"\" -DUSER_DB=\""$(pkgdatadir)/user.db"\" -DNUTRIKA_ICON=\""$(datadir)/pixmaps/nutrika.png"\"
bin_PROGRAMS = nutrika
nutrika_SOURCES = gui.c database.c database.h
nutrika_CFLAGS = $(GTK_CFLAGS) $(SQLITE_CFLAGS)
nutrika_LDADD = $(GTK_LIBS) $(SQLITE_LIBS)

