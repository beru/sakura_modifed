PLUGINS= \
	CSharpPlugin \
	CtagsPlugin \
	DiffLauncherPlugin \
	GlobalPlugin \
	HelloComplementPlugin \
	HelloPlugin \
	HunSpellChecker \
	RubyScriptPlugin

all:
	@for dir in $(PLUGINS); do \
		make -C $$dir; \
	done

clean:
	@for dir in $(PLUGINS); do \
		make clean -C $$dir; \
	done

