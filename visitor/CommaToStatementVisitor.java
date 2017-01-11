package visitor;

import ast.*;
import ast.expression.AssignmentExpression;
import ast.expression.impl.CommaExpression;
import ast.expression.Expression;
import ast.statement.Statement;
import ast.statement.impl.*;
import ast.statement.IterationStatement;
import lombok.Value;

import java.util.*;
import java.util.stream.Collectors;

/**
 * Created by ooee on 10/6/16.
 * <p>
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

    public Statement visit(Statement statement) {
        if (statement instanceof CompoundStatement) {
            return visit(((CompoundStatement) statement));
        } else if (statement instanceof IterationStatement) {
            return new CompoundStatement(visit((IterationStatement) statement));
        } else if (statement instanceof ExpressionStatement) {
            ExpressionStatement expressionStatement = (ExpressionStatement) statement;
            if (expressionStatement.getExpression() instanceof CommaExpression) {
                return new CompoundStatement(visit(((CommaExpression) expressionStatement.getExpression())));
            }
        }
        return statement;
    }

    public List<BlockItem> visit(ExpressionStatement statement) {
        Expression expression = statement.getExpression();
        if (expression instanceof CommaExpression) {
            return visit(((CommaExpression) expression));
        } else {
            return Arrays.asList(statement);
        }
    }

    public List<BlockItem> visit(CommaExpression expression) {
        List<AssignmentExpression> assignmentExpressions = ((CommaExpression) expression).getAssignmentExpressions();
        return assignmentExpressions.stream().map(exp -> new ExpressionStatement(exp)).collect(Collectors.toList());
    }

    public List<BlockItem> visit(IterationStatementWhile iterationStatementWhile) {
        Expression condition = iterationStatementWhile.getCondition();
        iterationStatementWhile.setStatement(visit(iterationStatementWhile.getStatement()));
        if (condition instanceof CommaExpression) {
            return processWhile(((CommaExpression) condition), iterationStatementWhile.getStatement());
        }
        return Arrays.asList(iterationStatementWhile);
    }

    public List<BlockItem> visit(IterationStatementDoWhile iterationStatementDoWhile) {
        Expression condition = iterationStatementDoWhile.getCondition();
        if (condition instanceof CommaExpression) {
            return processWhile(((CommaExpression) condition), iterationStatementDoWhile.getStatement());
        }
        return Arrays.asList(iterationStatementDoWhile);
    }

    private List<BlockItem> processWhile(CommaExpression condition, Statement body) {
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

    public List<BlockItem> visit(IterationStatementFor iterationStatementFor) {
        List<BlockItem> newBlockItems = new ArrayList<>();
        Expression initial = iterationStatementFor.getInitial();
        Expression condition = iterationStatementFor.getCondition();
        Expression iteration = iterationStatementFor.getIteration();
        iterationStatementFor.setStatement(visit(iterationStatementFor.getStatement()));
        Statement body = iterationStatementFor.getStatement();
        if (initial instanceof CommaExpression) {
            CommaComponent initialComponent = breakCommaExpression(((CommaExpression) initial));
            newBlockItems.addAll(initialComponent.getStatements());
            initial = initialComponent.getLastExp();
        }
        List<BlockItem> toAddToBody = new ArrayList<>();
        condition = processConditionExpression(condition, toAddToBody, newBlockItems);
        iteration = processIterationExpression(iteration, toAddToBody);
        for (BlockItem blockItem : toAddToBody) {
            CompoundStatement.addToEnd(blockItem, body);
        }
        newBlockItems.add(new IterationStatementFor(initial, condition, iteration, body));
        return newBlockItems;
    }

    public List<BlockItem> visit(IterationStatementDeclareFor iterationStatementDeclareFor) {
        Expression condition = iterationStatementDeclareFor.getCondition();
        Expression iteration = iterationStatementDeclareFor.getIteration();
        iterationStatementDeclareFor.setStatement(visit(iterationStatementDeclareFor.getStatement()));
        List<BlockItem> newBlockItems = new ArrayList<>();
        List<BlockItem> toAddToBody = new ArrayList<>();
        condition = processConditionExpression(condition, toAddToBody, newBlockItems);
        iteration = processIterationExpression(iteration, toAddToBody);

        Statement body = iterationStatementDeclareFor.getStatement();
        for (BlockItem blockItem : toAddToBody) {
            CompoundStatement.addToEnd(blockItem, body);
        }

        newBlockItems.add(new IterationStatementDeclareFor(iterationStatementDeclareFor.getVariableDeclaration(), condition, iteration, body));
        return newBlockItems;
    }

    public Expression processConditionExpression(Expression condition, List<BlockItem> toAddToBody, List<BlockItem> newBlockItems) {
        return condition;
        /*
        if (condition instanceof CommaExpression) {
            CommaComponent conditionComponent = breakCommaExpression(((CommaExpression) condition));
            for (ExpressionStatement expressionStatement : conditionComponent.getStatements()) {
                newBlockItems.add(expressionStatement);
                toAddToBody.add(expressionStatement);
            }
            return conditionComponent.getLastExp();
        }
        return condition;
        */
    }

    public Expression processIterationExpression(Expression iteration, List<BlockItem> toAddToBody) {
        if (iteration instanceof CommaExpression) {
            CommaComponent iterationComponent = breakCommaExpression(((CommaExpression) iteration));
            toAddToBody.addAll(iterationComponent.getStatements());
            return iterationComponent.getLastExp();
        } else {
            return iteration;
        }
    }


    public List<BlockItem> visit(IterationStatement statement) {
        if (statement instanceof IterationStatementWhile) {
            return visit(((IterationStatementWhile) statement));
        } else if (statement instanceof IterationStatementFor) {
            return visit(((IterationStatementFor) statement));
        } else if (statement instanceof IterationStatementDeclareFor) {
            return visit(((IterationStatementDeclareFor) statement));
        } else if (statement instanceof IterationStatementDoWhile) {
            return visit(((IterationStatementDoWhile) statement));
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
