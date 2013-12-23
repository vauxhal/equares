TEMPLATE = SUBDIRS

SUBDIRS =

folder_01.source = http
folder_01.target = .
EXTRAFOLDERS = folder_01

for(extrafolder, EXTRAFOLDERS) {
    item = item$${extrafolder}
    greaterThan(QT_MAJOR_VERSION, 4) {
        itemsources = $${item}.files
    } else {
        itemsources = $${item}.sources
    }
    $$itemsources = $$eval($${extrafolder}.source)
    itempath = $${item}.path
    $$itempath= $$eval($${extrafolder}.target)
    export($$itemsources)
    export($$itempath)
    DEPLOYMENT += $$item
}
