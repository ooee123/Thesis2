package ast;

import lombok.Value;
import pdg.PDGNode;

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
        if (expression instanceof Assigning) {
            Assigning assigning = (Assigning) expression;
            Set<String> usedVariables = assigning.getRightVariables();
            return usedVariables;
        }
        return expression.getVariables();
    }

    @Override
    public Set<String> getChangedVariables() {
        if (expression instanceof Assigning) {
            Assigning assigning = (Assigning) expression;
            Set<String> changedVariables = assigning.getLValues();
            return changedVariables;
        }
        return Collections.emptySet();
    }
}
