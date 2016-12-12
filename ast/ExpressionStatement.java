package ast;

import com.google.common.collect.Sets;
import lombok.Value;
import pdg.PDGUselessCodeRemover;
import visitor.Visitor;

import java.util.Collection;
import java.util.Collections;
import java.util.Set;

/**
 * Created by ooee on 9/25/16.
 */
@Value
public class ExpressionStatement implements Statement {
    private Expression expression;

    @Override
    public String toCode() {
        if (expression != null) {
            return expression.toCode() + ";";
        } else {
            return ";";
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
    public void visitAllExpressions(Visitor<Expression> visitor) {
        if (expression != null) {
            visitor.visit(expression);
        }
    }
}
