package ast.statement.impl;

import ast.expression.Expression;
import ast.statement.Statement;
import com.google.common.collect.Sets;
import lombok.AllArgsConstructor;
import lombok.Value;
import pdg.PDGUselessCodeRemover;
import visitor.Visitor;

import java.util.Set;

/**
 * Created by ooee on 9/25/16.
 */
@Value
@AllArgsConstructor
public class ExpressionStatement implements Statement {
    private Expression expression;
    private String originalLine;

    public ExpressionStatement(Expression expression) {
        this(expression, toCommentTip(expression));
    }

    @Override
    public String toCode(boolean showOriginalLine) {
        if (showOriginalLine) {
            return toCommentTip(expression) + "; /* " + originalLine + " */";
        } else {
            return toCommentTip(expression);
        }
    }

    private static String toCommentTip(Expression expression) {
        if (expression != null) {
            return expression.toCode();
        } else {
            return "";
        }
    }

    @Override
    public Set<String> getDependantVariables() {
        if (expression != null) {
            return expression.getDependentVariables();
        } else {
            return Sets.newHashSet();
        }
    }

    @Override
    public boolean isCritical() {
        if (expression != null) {
            return !expression.getInvocations().isEmpty() || (PDGUselessCodeRemover.TREAT_MEMORY_WRITES_AS_CRITICAL_AUTOMATICALLY && writesMemory());
        } else {
            return false;
        }
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        if (expression != null) {
            return expression.getGuaranteedChangedVariables();
        }
        return Sets.newHashSet();
    }

    @Override
    public Set<String> getPotentiallyChangedVariables() {
        if (expression != null) {
            return expression.getPotentiallyChangedVariables();
        } else {
            return Sets.newHashSet();
        }
    }

    @Override
    public void visitEachStatement(Visitor<Statement> visitor) {
        return;
    }

    @Override
    public void visitOwnedExpressions(Visitor<Expression> visitor) {
        if (expression != null) {
            visitor.visit(expression);
        }
    }

    @Override
    public void visitAllExpressions(Visitor<Expression> visitor) {
        visitOwnedExpressions(visitor);
    }
}
