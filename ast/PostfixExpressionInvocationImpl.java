package ast;

import lombok.Value;

import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class PostfixExpressionInvocationImpl implements PostfixExpression, Assigning {
    private PostfixExpression postfixExpression;
    private List<AssignmentExpression> arguments;

    @Override
    public Set<String> getLValues() {
        Set<String> lValues = new HashSet<>();
        for (AssignmentExpression argument : arguments) {
            if (argument instanceof AssignmentExpressionImpl) {
                lValues.addAll(argument.getLValues());
            }
        }
        return lValues;
    }

    public String toCode() {
        List<String> args = collectionToCode(arguments);
        return postfixExpression.toCode() + "(" + String.join(",", args) + ")";
    }

    @Override
    public Set<String> getVariables() {
        Set<String> variables = new HashSet<>();
        for (AssignmentExpression argument : arguments) {
            variables.addAll(argument.getVariables());
        }
        return variables;
    }

    @Override
    public Set<String> getRightVariables() {
        Set<String> rightVariables = new HashSet<>();
        for (AssignmentExpression argument : arguments) {
            if (argument instanceof Assigning) {
                rightVariables.addAll(((Assigning) argument).getRightVariables());
            }
        }
        return rightVariables;
    }
}
