all:
	javac -cp .:\* ast/*.java ast/type/*.java parser/*.java visitor/*.java

clean:
	rm *.class
