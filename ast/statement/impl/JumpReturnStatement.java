package ast.statement.impl;

import ast.expression.Expression;
import ast.statement.JumpStatementStrict;
import ast.statement.Statement;
import lombok.AllArgsConstructor;
import lombok.Value;
import visitor.Visitor;

import java.util.Set;

/**
 * Created by ooee on 9/25/16.
 */
@Value
@AllArgsConstructor
public class JumpReturnStatement implements JumpStatementStrict {
    private Expression returnExpression;
    private String originalLine;

    public JumpReturnStatement(Expression returnExpression) {
        this(returnExpression, toCommentTip(returnExpression));
    }

    @Override
    public String toCode(boolean showOriginalLine) {
        if (showOriginalLine) {
            return toCommentTip(returnExpression) + ";" + " /* " + originalLine + " */";
        } else {
            return toCommentTip(returnExpression) + ";";
        }
    }

    private static String toCommentTip(Expression returnExpression) {
        String returnString = "";
        if (returnExpression != null) {
            returnString = " " + returnExpression.toCode();
        }
        return "return" + returnString;
    }

    @Override
    public Set<String> getDependantVariables() {
        return returnExpression.getDependentVariables();
    }
/*
    @Override
    public Set<String> getChangedVariables() {
        return returnExpression.getGuaranteedChangedVariables();
    }
*/
    @Override
    public boolean isCritical() {
        return true;
    }

    @Override
    public Set<String> getPotentiallyChangedVariables() {
        return returnExpression.getPotentiallyChangedVariables();
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return returnExpression.getGuaranteedChangedVariables();
    }

    @Override
    public void visitEachStatement(Visitor<Statement> visitor) {
        return;
    }

    @Override
    public void visitOwnedExpressions(Visitor<Expression> visitor) {
        if (returnExpression != null) {
            visitor.visit(returnExpression);
        }
    }

    @Override
    public void visitAllExpressions(Visitor<Expression> visitor) {
        visitOwnedExpressions(visitor);
    }
}
