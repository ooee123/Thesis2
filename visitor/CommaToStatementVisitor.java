package visitor;

import ast.*;
import lombok.Value;
import org.antlr.v4.codegen.model.decl.Decl;

import java.util.*;
import java.util.stream.Collectors;

/**
 * Created by ooee on 10/6/16.
 *
 * Checks only for Commas inside CompoundStatements and Loops
 */
public class CommaToStatementVisitor {
    @Value
    private class CommaComponent {
        private List<ExpressionStatement> statements;
        private AssignmentExpression lastExp;
    }

    public Program visit(Program p) {
        Collection<Function> functions = p.getFunction();
        for (Function function : functions) {
            function.setCompoundStatement(visit(function.getCompoundStatement()));
        }
        return p;
    }

    public CompoundStatement visit(CompoundStatement statement) {
        List<BlockItem> blockItems = statement.getBlockItems();
        List<BlockItem> newBlockItems = new ArrayList<>();
        for (BlockItem blockItem : blockItems) {
            if (blockItem instanceof ExpressionStatement) {
                newBlockItems.addAll(visit(((ExpressionStatement) blockItem)));
            } else if (blockItem instanceof IterationStatement) {
                newBlockItems.addAll(visit(((IterationStatement) blockItem)));
            } else {
                newBlockItems.add(blockItem);
            }
        }
        return new CompoundStatement(newBlockItems);
    }

    public List<BlockItem> visit(ExpressionStatement statement) {
        Expression expression = statement.getExpression();
        if (expression instanceof CommaExpression) {
            List<AssignmentExpression> assignmentExpressions = ((CommaExpression) expression).getAssignmentExpressions();
            return assignmentExpressions.stream().map(exp -> new ExpressionStatement(exp)).collect(Collectors.toList());
        } else {
            return Arrays.asList(statement);
        }
    }

    public List<BlockItem> visit(IterationStatement statement) {
        if (statement instanceof IterationStatementWhile) {
            IterationStatementWhile iterationStatementWhile = (IterationStatementWhile) statement;

            Expression condition = iterationStatementWhile.getCondition();
            if (condition instanceof CommaExpression) {
                Statement body = iterationStatementWhile.getStatement();
                List<BlockItem> newBlockItems = new ArrayList<>();
                CommaComponent commaComponent = breakCommaExpression(((CommaExpression) condition));
                newBlockItems.addAll(commaComponent.getStatements());
                for (ExpressionStatement conditionStatement : commaComponent.getStatements()) {
                    CompoundStatement.addToEnd(conditionStatement, body);
                }
                Expression last = commaComponent.getLastExp();
                newBlockItems.add(new IterationStatementWhile(last, body));
                return newBlockItems;
            }
        } else if (statement instanceof IterationStatementFor) {
            IterationStatementFor iterationStatementFor = (IterationStatementFor) statement;
            List<BlockItem> newBlockItems = new ArrayList<>();
            Expression initial = iterationStatementFor.getInitial();
            Expression condition = iterationStatementFor.getCondition();
            Expression iteration = iterationStatementFor.getIteration();

            Statement body = iterationStatementFor.getStatement();
            if (initial instanceof CommaExpression) {
                CommaComponent initialComponent = breakCommaExpression(((CommaExpression) initial));
                for (ExpressionStatement expressionStatement : initialComponent.getStatements()) {
                    newBlockItems.add(expressionStatement);
                }
                initial = initialComponent.getLastExp();
            }
            List<BlockItem> toAddToBody = new ArrayList<>();

            /* Can't refactor condition, because operations may go out of order.
            /*
            if (condition instanceof CommaExpression) {
                List<BlockItem> toAddToBodyComma = new ArrayList<>();
                CommaComponent conditionComponent = breakCommaExpression(((CommaExpression) condition));
                for (ExpressionStatement expressionStatement : conditionComponent.getStatements()) {
                    newBlockItems.add(expressionStatement);
                    toAddToBodyComma.add(expressionStatement);
                }
                condition = conditionComponent.getLastExp();
                toAddToBody.addAll(toAddToBodyComma);
            }
            */

            if (iteration instanceof CommaExpression) {
                CommaComponent iterationComponent = breakCommaExpression(((CommaExpression) iteration));
                List<BlockItem> toAddToBodyIter = new ArrayList<>();
                for (ExpressionStatement expressionStatement : iterationComponent.getStatements()) {
                    toAddToBodyIter.add(expressionStatement);
                }
                iteration = iterationComponent.getLastExp();
                toAddToBody.addAll(0, toAddToBodyIter);
            }
            for (BlockItem blockItem : toAddToBody) {
                CompoundStatement.addToEnd(blockItem, body);
            }
            newBlockItems.add(new IterationStatementFor(initial, condition, iteration, body));
            return newBlockItems;
        } else if (statement instanceof IterationStatementDeclareFor) {
            IterationStatementDeclareFor iterationStatementDeclareFor = (IterationStatementDeclareFor) statement;
            Expression condition = iterationStatementDeclareFor.getCondition();
            Expression iteration = iterationStatementDeclareFor.getIteration();

            Statement body = iterationStatementDeclareFor.getStatement();
            List<BlockItem> toAddToBody = new ArrayList<>();

            /* Can't refactor condition, because operations may go out of order.
            /*
            if (condition instanceof CommaExpression) {
                List<BlockItem> toAddToBodyComma = new ArrayList<>();
                CommaComponent conditionComponent = breakCommaExpression(((CommaExpression) condition));
                for (ExpressionStatement expressionStatement : conditionComponent.getStatements()) {
                    newBlockItems.add(expressionStatement);
                    toAddToBodyComma.add(expressionStatement);
                }
                condition = conditionComponent.getLastExp();
                toAddToBody.addAll(toAddToBodyComma);
            }
            */

            if (iteration instanceof CommaExpression) {
                CommaComponent iterationComponent = breakCommaExpression(((CommaExpression) iteration));
                List<BlockItem> toAddToBodyIter = new ArrayList<>();
                for (ExpressionStatement expressionStatement : iterationComponent.getStatements()) {
                    toAddToBodyIter.add(expressionStatement);
                }
                iteration = iterationComponent.getLastExp();
                toAddToBody.addAll(0, toAddToBodyIter);
            }
            for (BlockItem blockItem : toAddToBody) {
                CompoundStatement.addToEnd(blockItem, body);
            }
            List<BlockItem> newBlockItems = new ArrayList<>();
            newBlockItems.add(new IterationStatementDeclareFor(iterationStatementDeclareFor.getDeclaration(), condition, iteration, body));
            return newBlockItems;
        } else if (statement instanceof IterationStatementDoWhile) {
            IterationStatementDoWhile iterationStatementDoWhile = (IterationStatementDoWhile) statement;
            Expression condition = iterationStatementDoWhile.getCondition();
            if (condition instanceof CommaExpression) {
                Statement body = iterationStatementDoWhile.getStatement();
                List<BlockItem> newBlockItems = new ArrayList<>();
                CommaComponent commaComponent = breakCommaExpression(((CommaExpression) condition));
                newBlockItems.addAll(commaComponent.getStatements());
                for (ExpressionStatement conditionStatement : commaComponent.getStatements()) {
                    CompoundStatement.addToEnd(conditionStatement, body);
                }
                Expression last = commaComponent.getLastExp();
                newBlockItems.add(new IterationStatementWhile(last, body));
                return newBlockItems;
            }
        }
        return Arrays.asList(statement);
    }

    public CommaComponent breakCommaExpression(CommaExpression exp) {
        List<AssignmentExpression> assignmentExpressions = exp.getAssignmentExpressions();
        AssignmentExpression last = assignmentExpressions.remove(assignmentExpressions.size() - 1);
        List<ExpressionStatement> expressionStatements = assignmentExpressions.stream().map(asnExp -> new ExpressionStatement(asnExp)).collect(Collectors.toList());
        return new CommaComponent(expressionStatements, last);
    }
}
