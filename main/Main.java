package main;

import IdentifierNormalizer.ProgramIdentifierNormalizerVisitor;
import ast.Function;
import ast.Program;
import ast.statement.impl.CompoundStatement;
import org.antlr.v4.runtime.ANTLRInputStream;
import org.antlr.v4.runtime.CommonTokenStream;
import parser.CLexer;
import parser.CParser;
import pdg.*;
import visitor.PDGGenerationVisitor;
import visitor.PrimaryExpressionTypeAssignerVisitor;
import visitor.TreeToASTVisitor;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.IOException;

/**
 * Created by ooee on 10/20/16.
 */
public class Main {
    public static void main(String args[]) {
        if (args.length < 1) {
            System.err.println("Put C file as first parameter");
            System.exit(-1);
        }
        try {
            ByteArrayInputStream byteArrayInputStream = new ByteArrayInputStream(Preprocess.preprocess(new File(args[0])).getBytes());
            CLexer lexer = new CLexer(new ANTLRInputStream(byteArrayInputStream));
            CommonTokenStream tokens = new CommonTokenStream(lexer);
            System.err.println("Begin parsing");
            CParser parser = new CParser(tokens);
            System.err.println("Finish parsing");
            CParser.CompilationUnitContext compilationUnit = parser.compilationUnit();
            TreeToASTVisitor visitor = new TreeToASTVisitor();
            System.err.println("Begin AST building");
            Program program = visitor.visit(compilationUnit);
            System.err.println("Finish AST building");

            System.err.println("Assigning types to primary expressions");
            new PrimaryExpressionTypeAssignerVisitor(program);
            System.err.println("Finish assigning types to primary expressions");

            PDGGenerationVisitor pdgGenerationVisitor = new PDGGenerationVisitor(program);

            //pdgGenerationVisitor.visit(program);
            //FunctionArgumentOrderVisitor functionArgumentOrderVisitor = new FunctionArgumentOrderVisitor();
            //functionArgumentOrderVisitor.visit(program);
            for (Function function : program.getFunction()) {
                System.err.println("Begin PDG generation for function " + function.getIdentifier());
                PDGNodeCompoundStatement functionBody = pdgGenerationVisitor.visit(function);
                System.err.println("Finish PDG generation");
                PDGUselessCodeRemover pdgUselessCodeRemover = new PDGUselessCodeRemover();
                pdgUselessCodeRemover.removeUselessCode(functionBody);

                PDGNodeTransitiveReducer pdgNodeTransitiveReducer = new PDGNodeTransitiveReducer();
                pdgNodeTransitiveReducer.reduce(functionBody);

                PDGSorter sorter = new PDGSorterDefault();
                CompoundStatement statement = sorter.sort(functionBody.getBody());
                function.setCompoundStatement(statement);
            }
            ProgramIdentifierNormalizerVisitor programIdentifierNormalizerVisitor = new ProgramIdentifierNormalizerVisitor();
            programIdentifierNormalizerVisitor.visit(program);
            System.out.println(program.toCode());
        } catch (IOException e) {
            System.err.println(e);
        }
    }
}
