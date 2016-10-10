package ast;

import lombok.Value;

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class PostfixExpressionArrayAccessImpl implements PostfixExpression, Assigning {
    private PostfixExpression postfixExpression;
    private Expression arrayIndex;

    @Override
    public Set<String> getLValues() {
        return postfixExpression.getLValues();
    }

    public String toCode() {
        return postfixExpression.toCode() + "[" + arrayIndex.toCode() + "]";
    }

    @Override
    public Set<String> getVariables() {
        Set<String> variables = new HashSet<>();
        variables.addAll(postfixExpression.getVariables());
        variables.addAll(arrayIndex.getVariables());
        return variables;
    }

    @Override
    public Set<String> getRightVariables() {
        if (arrayIndex instanceof Assigning) {
            return ((Assigning) arrayIndex).getRightVariables();
        } else {
            return Collections.emptySet();
        }
    }
}
