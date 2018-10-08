install-game-data:
	$(INSTALL) -d $(DESTDIR)${game_datadir}/image/score
	for f in *.ppm; do \
		$(install_sh_DATA) $${f} $(DESTDIR)${game_datadir}/image/score/$${f}; \
	done
	for f in *.epm; do \
		$(install_sh_DATA) $${f} $(DESTDIR)${game_datadir}/image/score/$${f}; \
	done

