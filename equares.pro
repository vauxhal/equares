TEMPLATE = subdirs

SUBDIRS += \
    equares_core \
    equares_con \
    equares_gui
# \
#    equares_http

equares_con.depends += equares_core
equares_gui.depends += equares_core
