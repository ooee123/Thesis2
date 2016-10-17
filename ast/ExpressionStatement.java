package ast;

import lombok.Value;

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
        return expression.toCode() + ";";
    }

    @Override
    public Set<String> getDependantVariables() {
        return expression.getDependentVariables();
    }

    @Override
    public Set<String> getChangedVariables() {
        return expression.getChangedVariables();
    }

    @Override
    public boolean isCritical() {
        return !expression.getInvocations().isEmpty();
    }
}
