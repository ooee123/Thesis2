package tst;

import ast.*;
import ast.statement.impl.CompoundStatement;
import org.antlr.v4.runtime.ANTLRFileStream;
import org.antlr.v4.runtime.CommonTokenStream;
import org.junit.Test;
import parser.CLexer;
import parser.CParser;
import pdg.PDGNodeCompoundStatement;
import pdg.PDGSorterDefault;
import visitor.PDGGenerationVisitor;
import visitor.TreeToASTVisitor;

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
        PDGGenerationVisitor pdgVisitor = new PDGGenerationVisitor(program);
        for (Function function : program.getFunction()) {
            PDGNodeCompoundStatement functionBody = pdgVisitor.visit(function);
            PDGSorterDefault sorter = new PDGSorterDefault();
            CompoundStatement statement = sorter.sort(functionBody.getBody());
            function.setCompoundStatement(statement);
            System.out.println("Reprinted program");
            System.out.println(function.toCode(false));
        }


    }
}