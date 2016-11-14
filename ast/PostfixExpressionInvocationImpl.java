package ast;

import com.google.common.collect.Sets;
import lombok.AllArgsConstructor;
import lombok.Data;
import visitor.Visitor;

import java.util.*;

/**
 * Created by ooee on 9/26/16.
 */
@Data
@AllArgsConstructor
public class PostfixExpressionInvocationImpl implements PostfixExpression {
    private PostfixExpression postfixExpression;
    private List<AssignmentExpression> arguments;
    private Function function;

    public PostfixExpressionInvocationImpl(PostfixExpression expression, List<AssignmentExpression> arguments) {
        this.postfixExpression = expression;
        this.arguments = arguments;
    }

    public boolean writesMemory() {
        if (function != null) {
            return function.getCompoundStatement().writesMemory();
        } else {
            return true;
        }
    }

    public boolean readsMemory() {
        if (function != null) {
            return function.getCompoundStatement().readsMemory();
        } else {
            return true;
        }
    }

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
        return postfixExpression.toCode() + "(" + String.join(", ", args) + ")";
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
        Set<PostfixExpressionInvocationImpl> invocations = Sets.newIdentityHashSet();
        invocations.add(this);
        invocations.addAll(multiGetInvocations(arguments.toArray(new Expression[0])));
        return invocations;
    }

    @Override
    public void visitNestedExpressions(Visitor<Expression> visitor) {
        visitor.visit(postfixExpression);
        for (AssignmentExpression argument : arguments) {
            visitor.visit(argument);
        }
    }
}
