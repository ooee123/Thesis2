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
    public static final boolean SHOW_ORIGINAL_LINE = false;

    public static void main(String args[]) {
        if (args.length < 1) {
            System.err.println("Put C file as first parameter");
            System.exit(-1);
        }
        try {
            String preprocess = Preprocess.preprocess(new File(args[0]));
            int splittingIndex = Preprocess.getSplittingIndex(preprocess);
            String firstPart = Preprocess.removePounds(preprocess.substring(0, splittingIndex));
            String secondPart = Preprocess.removePounds(preprocess.substring(splittingIndex, preprocess.length()));
            //ByteArrayInputStream firstPartByteArrayInputStream = new ByteArrayInputStream(Preprocess.preprocess(new File(args[0])).getBytes());
            ByteArrayInputStream firstPartByteArrayInputStream = new ByteArrayInputStream(firstPart.getBytes());
            ByteArrayInputStream secondPartByteArrayInputStream = new ByteArrayInputStream(secondPart.getBytes());
            CLexer firstPartlexer = new CLexer(new ANTLRInputStream(firstPartByteArrayInputStream));
            CLexer secondPartlexer = new CLexer(new ANTLRInputStream(secondPartByteArrayInputStream));
            CommonTokenStream firstPartTokens = new CommonTokenStream(firstPartlexer);
            CommonTokenStream secondPartTokens = new CommonTokenStream(secondPartlexer);
            System.err.println("Begin parsing");
            CParser firstPartParser = new CParser(firstPartTokens);
            CParser secondPartParser = new CParser(secondPartTokens);
            System.err.println("Finish parsing");
            CParser.CompilationUnitContext firstPartCompilationUnit = firstPartParser.compilationUnit();
            CParser.CompilationUnitContext secondPartCompilationUnit = secondPartParser.compilationUnit();
            TreeToASTVisitor visitor = new TreeToASTVisitor(firstPartCompilationUnit, secondPartTokens);
            System.err.println("Begin AST building");
            Program program = visitor.visit(secondPartCompilationUnit);
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
            System.out.println(program.toCode(SHOW_ORIGINAL_LINE));
        } catch (IOException e) {
            System.err.println(e);
        }
    }
}
