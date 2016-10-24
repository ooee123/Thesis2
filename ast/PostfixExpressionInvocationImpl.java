package ast;

import lombok.AllArgsConstructor;
import lombok.Data;
import visitor.Visitor;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * Created by ooee on 9/26/16.
 */
@Data
@AllArgsConstructor
public class PostfixExpressionInvocationImpl implements PostfixExpression {
    private PostfixExpression postfixExpression;
    private List<AssignmentExpression> arguments;

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        Set<String> changedVariables = new HashSet<>();
        for (AssignmentExpression argument : arguments) {
            changedVariables.addAll(argument.getGuaranteedChangedVariables());
        }
        if (postfixExpression instanceof PrimaryExpressionIdentifier) {
            String functionName = ((PrimaryExpressionIdentifier) postfixExpression).getIdentifier();
            //TODO: Get pointer arguments, and add them to changedVariables
        } else if (postfixExpression instanceof PostfixExpressionStructAccessImpl) {
            System.err.println("Invoking a function as a member field");
        }
        return changedVariables;
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
    public List<PostfixExpressionInvocationImpl> getInvocations() {
        List<PostfixExpressionInvocationImpl> invocations = new ArrayList<>();
        invocations.add(this);
        invocations.addAll(multiGetInvocations(arguments.toArray(new Expression[0])));
        return invocations;
    }

    @Override
    public void visitNestedExpressions(Visitor<Void, Expression> visitor) {
        visitor.visit(postfixExpression);
        for (AssignmentExpression argument : arguments) {
            visitor.visit(argument);
        }
    }
}
