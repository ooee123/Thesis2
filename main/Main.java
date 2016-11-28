package main;

import IdentifierNormalizer.ProgramIdentifierNormalizerVisitor;
import ast.*;
import org.antlr.v4.runtime.ANTLRInputStream;
import org.antlr.v4.runtime.CommonTokenStream;
import parser.CLexer;
import parser.CParser;
import pdg.*;
import visitor.FunctionArgumentOrderVisitor;
import IdentifierNormalizer.FunctionIdentifierNormalizerVisitor;
import visitor.PDGGenerationVisitor;
import visitor.TreeToASTVisitor;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.IOException;
import java.util.Scanner;

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
            ByteArrayInputStream byteArrayInputStream = new ByteArrayInputStream(Preprocess.proprocess(new File(args[0])).getBytes());
            CLexer lexer = new CLexer(new ANTLRInputStream(byteArrayInputStream));
            CommonTokenStream tokens = new CommonTokenStream(lexer);
            CParser parser = new CParser(tokens);
            CParser.CompilationUnitContext compilationUnit = parser.compilationUnit();
            TreeToASTVisitor visitor = new TreeToASTVisitor();
            Program program = visitor.visit(compilationUnit);
            PDGGenerationVisitor pdgGenerationVisitor = new PDGGenerationVisitor(program);
            //pdgGenerationVisitor.visit(program);
            FunctionArgumentOrderVisitor functionArgumentOrderVisitor = new FunctionArgumentOrderVisitor();
            //functionArgumentOrderVisitor.visit(program);
            for (Function function : program.getFunction()) {
                PDGNodeCompoundStatement functionBody = pdgGenerationVisitor.visit(function);
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
