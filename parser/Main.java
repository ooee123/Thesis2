package parser;

import org.antlr.v4.runtime.ANTLRFileStream;
import org.antlr.v4.runtime.CommonTokenStream;
import visitor.TreeToASTVisitor;

import java.io.IOException;

/**
 * Created by ooee on 9/24/16.
 */
public class Main {
    public static void main(String[] args) throws IOException {
        CLexer lexer = new CLexer(new ANTLRFileStream(args[0]));
        CommonTokenStream tokens = new CommonTokenStream(lexer);
        CParser parser = new CParser(tokens);

        CParser.CompilationUnitContext compilationUnit = parser.compilationUnit();
        TreeToASTVisitor visitor = new TreeToASTVisitor();
        visitor.visit(compilationUnit);
    }
}
