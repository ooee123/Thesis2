package ast;

import lombok.Value;

import java.util.HashSet;
import java.util.Set;

/**
 * Created by ooee on 10/6/16.
 */
@Value
public class LabeledIdentifierStatement implements LabeledStatement {
    private String identifier;
    private Statement statement;

    @Override
    public String toCode() {
        return identifier + ": " + statement.toCode();
    }

    @Override
    public Set<String> getDependantVariables() {
        return statement.getDependantVariables();
    }

    @Override
    public Set<String> getChangedVariables() {
        return statement.getChangedVariables();
    }

    @Override
    public boolean isCritical() {
        return false;
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return statement.getGuaranteedChangedVariables();
    }

    @Override
    public Set<String> getPotentiallyChangedVariables() {
        return statement.getPotentiallyChangedVariables();
    }
}
