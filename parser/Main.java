package parser;

import ast.Program;
import org.antlr.v4.runtime.ANTLRFileStream;
import org.antlr.v4.runtime.CommonTokenStream;
import visitor.LoopsToWhileVisitor;
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
        Program program = visitor.visit(compilationUnit);
        System.out.println(program.toCode());


        System.out.println("FINISH PARSING");

        LoopsToWhileVisitor loopsToWhileVisitor = new LoopsToWhileVisitor();
        program = loopsToWhileVisitor.visit(program);
        System.out.print(program.toCode());
    }
}
