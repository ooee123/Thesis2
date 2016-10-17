package ast;

import lombok.Value;

import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class CommaExpression implements Expression {
    private List<AssignmentExpression> assignmentExpressions;

    @Override
    public String toCode() {
        List<String> expCodes = assignmentExpressions.stream().map(exp -> exp.toCode()).collect(Collectors.toList());
        return String.join(", ", expCodes);
    }

    @Override
    public Set<String> getChangedVariables() {
        return Collections.emptySet();
    }

    @Override
    public Set<String> getDependentVariables() {
        Set<String> rightVariables = new HashSet<>();
        for (AssignmentExpression assignmentExpression : assignmentExpressions) {
            rightVariables.addAll(( assignmentExpression).getDependentVariables());
        }
        return rightVariables;
    }

    @Override
    public Set<String> getVariables() {
        Set<String> variables = new HashSet<>();
        for (AssignmentExpression assignmentExpression : assignmentExpressions) {
            variables.addAll(assignmentExpression.getVariables());
        }
        return variables;
    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        Set<PostfixExpressionInvocationImpl> invocations = new HashSet<>();
        for (AssignmentExpression assignmentExpression : assignmentExpressions) {
            invocations.addAll(assignmentExpression.getInvocations());
        }
        return invocations;
    }
}
