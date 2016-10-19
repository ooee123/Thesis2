package visitor;

import ast.Program;
import org.antlr.v4.runtime.ANTLRFileStream;
import org.antlr.v4.runtime.CommonTokenStream;
import org.junit.Test;
import parser.CLexer;
import parser.CParser;
import pdg.PDGSorterDefault;

/**
 * Created by ooee on 10/8/16.
 */
public class ASTGeneralVisitorTest {
    public static final String TEST_FILE = "cfiles/lValueTest.c";
    @Test
    public void test() throws Exception {
        CLexer lexer = new CLexer(new ANTLRFileStream(TEST_FILE));
        CommonTokenStream tokens = new CommonTokenStream(lexer);
        CParser parser = new CParser(tokens);

        CParser.CompilationUnitContext compilationUnit = parser.compilationUnit();
        TreeToASTVisitor visitor = new TreeToASTVisitor();
        Program program = visitor.visit(compilationUnit);
        PDGGenerationVisitor pdgVisitor = new PDGGenerationVisitor();
        pdgVisitor.visit(program);
        System.out.println(pdgVisitor.getPDGNodes().size());
        PDGSorterDefault sorter = new PDGSorterDefault(pdgVisitor.getPDGNodes());
        System.out.println("Reprinted program");
        sorter.process();
    }
}