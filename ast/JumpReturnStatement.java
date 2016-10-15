package ast;

import lombok.Value;

import java.util.Collections;
import java.util.Set;

/**
 * Created by ooee on 9/25/16.
 */
@Value
public class JumpReturnStatement implements JumpStatement {
    private Expression returnExpression;

    @Override
    public String toCode() {
        return "return " + returnExpression.toCode() + ";";
    }

    @Override
    public Set<String> getDependantVariables() {
        if (returnExpression instanceof Assigning) {
            return ((Assigning) returnExpression).getRightVariables();
        } else {
            return returnExpression.getVariables();
        }
    }

    @Override
    public Set<String> getChangedVariables() {
        if (returnExpression instanceof Assigning) {
            return returnExpression.getLValues();
        } else {
            return Collections.emptySet();
        }
    }
}
