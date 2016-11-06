package visitor;

import ast.*;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

/**
 * Created by ooee on 10/6/16.
 */
public class LoopsToWhileVisitor {
    public Program visit(Program program) {
        Collection<Function> functions = program.getFunction();
        for (Function function : functions) {
            function.setCompoundStatement((CompoundStatement)visit(function.getCompoundStatement()));
        }
        return program;
    }

    public Statement visit(Statement statement) {
        if (statement instanceof CompoundStatement) {
            CompoundStatement compoundStatement = (CompoundStatement) statement;
            List<BlockItem> blockItems = compoundStatement.getBlockItems();
            List<BlockItem> newBlockItems = new ArrayList<>();
            for (BlockItem blockItem : blockItems) {
                if (blockItem instanceof IterationStatement) {
                    List<BlockItem> blockifiedIteration = iterationToWhile(((IterationStatement) blockItem));
                    newBlockItems.addAll(blockifiedIteration);
                } else {
                    if (blockItem instanceof Statement) {
                        newBlockItems.add(visit((Statement) blockItem));
                    } else {
                        newBlockItems.add(blockItem);
                    }
                }
            }
            return new CompoundStatement(newBlockItems);
        } else if (statement instanceof IterationStatement) {
            List<BlockItem> newBlockItems = iterationToWhile(((IterationStatement) statement));
            return new CompoundStatement(newBlockItems);
        } else if (statement instanceof SelectionStatementIf) {
            SelectionStatementIf selectionStatementIf = (SelectionStatementIf) statement;
            selectionStatementIf.setThenStatement(visit(selectionStatementIf.getThenStatement()));
            selectionStatementIf.setElseStatement(visit(selectionStatementIf.getElseStatement()));
            return selectionStatementIf;
        } else if (statement instanceof SelectionStatementSwitch) {
            SelectionStatementSwitch selectionStatementSwitch = (SelectionStatementSwitch) statement;
            selectionStatementSwitch.setStatement(visit(selectionStatementSwitch.getStatement()));
            return selectionStatementSwitch;
        }
        return statement;
    }

    public List<BlockItem> iterationToWhile(IterationStatement iterationStatement) {
        if (iterationStatement instanceof IterationStatementFor) {
            return iterationToWhile(((IterationStatementFor) iterationStatement));
        } else if (iterationStatement instanceof IterationStatementDeclareFor) {
            return iterationToWhile(((IterationStatementDeclareFor) iterationStatement));
        } else if (iterationStatement instanceof IterationStatementDoWhile) {
            return iterationToWhile(((IterationStatementDoWhile) iterationStatement));
        } else {
            return Arrays.asList(iterationStatement);
        }
    }

    public List<BlockItem> iterationToWhile(IterationStatementFor iterationStatementFor) {
        BlockItem initialStatement = new ExpressionStatement(iterationStatementFor.getInitial());
        Statement iterStatement = new ExpressionStatement(iterationStatementFor.getIteration());
        Statement body = visit(iterationStatementFor.getStatement());

        body = CompoundStatement.addToEnd(iterStatement, body);
        BlockItem whileStatement = new IterationStatementWhile(iterationStatementFor.getCondition(), body);

        List<BlockItem> blockItems = new ArrayList<>();
        blockItems.add(initialStatement);
        blockItems.add(whileStatement);
        return blockItems;
    }

    public List<BlockItem> iterationToWhile(IterationStatementDeclareFor iterationStatementDeclareFor) {
        List<BlockItem> blockItems = new ArrayList<>();
        blockItems.add(iterationStatementDeclareFor.getVariableDeclaration());
        Statement iterStatement = new ExpressionStatement(iterationStatementDeclareFor.getIteration());
        Statement body = visit(iterationStatementDeclareFor.getStatement());
        body = CompoundStatement.addToEnd(iterStatement, body);

        BlockItem whileStatement = new IterationStatementWhile(iterationStatementDeclareFor.getCondition(), body);

        blockItems.add(whileStatement);
        return blockItems;
    }

    public List<BlockItem> iterationToWhile(IterationStatementDoWhile iterationStatementDoWhile) {
        List<BlockItem> blockItems = new ArrayList<>();
        Statement statement = iterationStatementDoWhile.getStatement();
        if (statement instanceof CompoundStatement) {
            blockItems.addAll(((CompoundStatement) statement).getBlockItems());
        } else {
            blockItems.add(statement);
        }
        blockItems.add(new IterationStatementWhile(iterationStatementDoWhile.getCondition(), iterationStatementDoWhile.getStatement()));
        return blockItems;
    }
}