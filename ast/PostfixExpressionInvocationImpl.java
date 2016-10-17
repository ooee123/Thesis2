package ast;

import lombok.Value;

import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class PostfixExpressionInvocationImpl implements PostfixExpression {
    private PostfixExpression postfixExpression;
    private List<AssignmentExpression> arguments;

    @Override
    public Set<String> getChangedVariables() {
        Set<String> lValues = new HashSet<>();
        for (AssignmentExpression argument : arguments) {
            lValues.addAll(argument.getChangedVariables());
        }
        if (postfixExpression instanceof PrimaryExpressionIdentifier) {
            String functionName = ((PrimaryExpressionIdentifier) postfixExpression).getIdentifier();
            //TODO: Get arguments that change
            //Set<String> changedVariables = getChangedVariables(arguments);
            Set<String> changedVariables = new HashSet<>();
            for (AssignmentExpression argument : arguments) {
                changedVariables.addAll(argument.getVariables());
            }
            lValues.addAll(changedVariables);
        } else if (postfixExpression instanceof PostfixExpressionStructAccessImpl) {
            System.err.println("Invoking a function as a member field");
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
    public Set<String> getDependentVariables() {
        Set<String> rightVariables = new HashSet<>();
        for (AssignmentExpression argument : arguments) {
            rightVariables.addAll(argument.getDependentVariables());
        }
        return rightVariables;
    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        Set<PostfixExpressionInvocationImpl> invocations = new HashSet<>();
        invocations.add(this);
        invocations.addAll(multiGetInvocations(arguments.toArray(new Expression[0])));
        return invocations;
    }
}
