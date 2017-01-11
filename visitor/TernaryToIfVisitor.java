package visitor;

import ast.*;
import ast.expression.impl.ConditionalExpressionImpl;
import ast.statement.Statement;
import ast.statement.impl.ExpressionStatement;
import ast.statement.impl.SelectionStatementIf;

/**
 * Created by ooee on 10/6/16.
 */
public class TernaryToIfVisitor {
    public Program visit(Program p) {
        return p;
    }

    public SelectionStatementIf visit(ConditionalExpressionImpl exp) {
        Statement falseStatement = null;
        if (exp.getFalseExpression() != null) {
            falseStatement = new ExpressionStatement(exp.getFalseExpression());
        }
        return new SelectionStatementIf(exp.getLogicalOrExpression(), new ExpressionStatement(exp.getTrueExpression()), falseStatement);
    }
}
