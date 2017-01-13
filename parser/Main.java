package parser;

import ast.Program;
import org.antlr.v4.runtime.ANTLRFileStream;
import org.antlr.v4.runtime.CommonTokenStream;
import visitor.*;

import java.io.IOException;
import java.util.*;

/**
 * Created by ooee on 9/24/16.
 */
public class Main {
    public static boolean SHOW_ORIGINAL_LINES = true;

    public static void main(String[] args) throws IOException {

        boolean loopsToWhileVisitorFlag = false;
        boolean ternaryToIfFlag = false;
        boolean commaToStatementFlag = false;
        boolean printParsedProgramFlag = false;

        Map<String, String> options = new LinkedHashMap<>();
        options.put("-l", "Loops to while");
        options.put("-if", "Ternary to if (NOT SUPPORTED)");
        options.put("-c", "Commas to Statements");
        options.put("-p", "Print parsed program");

        List<String> strings = Arrays.asList(args);
        if (strings.contains("-h")) {
            for (Map.Entry<String, String> stringStringEntry : options.entrySet()) {
                System.out.println(stringStringEntry.getKey() + "\t" + stringStringEntry.getValue());
            }
            System.exit(0);
        }
        loopsToWhileVisitorFlag = strings.contains("-l");
        ternaryToIfFlag = strings.contains("-if");
        commaToStatementFlag = strings.contains("-c");
        printParsedProgramFlag = strings.contains("-p");

        CLexer lexer = new CLexer(new ANTLRFileStream(args[0]));
        CommonTokenStream tokens = new CommonTokenStream(lexer);
        CParser parser = new CParser(tokens);

        CParser.CompilationUnitContext compilationUnit = parser.compilationUnit();
        TreeToASTVisitor visitor = new TreeToASTVisitor();
        Program program = visitor.visit(compilationUnit);
        if (printParsedProgramFlag) {
            System.out.println(program.toCode(SHOW_ORIGINAL_LINES));
        }

        ASTGeneralVisitor astGeneralVisitor = new ASTGeneralVisitor(program);
        //System.out.println(astGeneralVisitor.diagnose());
        astGeneralVisitor.walk();

        if (loopsToWhileVisitorFlag) {
            LoopsToWhileVisitor loopsToWhileVisitor = new LoopsToWhileVisitor();
            program = loopsToWhileVisitor.visit(program);
        }
        if (ternaryToIfFlag) {
            TernaryToIfVisitor ternaryToIfVisitor = new TernaryToIfVisitor();
            program = ternaryToIfVisitor.visit(program);
        }
        if (commaToStatementFlag) {
            CommaToStatementVisitor commaToStatementVisitor = new CommaToStatementVisitor();
            program = commaToStatementVisitor.visit(program);
        }
        System.out.print(program.toCode(SHOW_ORIGINAL_LINES));
    }
}
