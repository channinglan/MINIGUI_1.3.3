# The directory where the data files will be installed

install-data-local:
	$(INSTALL) -d $(prefix)/etc

if FLAT_WINDOW_STYLE
	$(INSTALL_DATA) MiniGUI-flat.cfg $(prefix)/etc/MiniGUI.cfg
else
	$(INSTALL_DATA) MiniGUI-3d.cfg $(prefix)/etc/MiniGUI.cfg
endif

EXTRA_DIST = MiniGUI-flat.cfg MiniGUI-3d.cfg

