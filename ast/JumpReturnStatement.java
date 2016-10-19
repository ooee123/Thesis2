package ast;

import lombok.Value;

import java.util.Set;

/**
 * Created by ooee on 9/25/16.
 */
@Value
public class JumpReturnStatement implements JumpStatementStrict {
    private Expression returnExpression;

    @Override
    public String toCode() {
        return "return " + returnExpression.toCode() + ";";
    }

    @Override
    public Set<String> getDependantVariables() {
        return returnExpression.getDependentVariables();
    }

    @Override
    public Set<String> getChangedVariables() {
        return returnExpression.getGuaranteedChangedVariables();
    }

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
}
