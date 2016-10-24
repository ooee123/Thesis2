package ast;

import com.google.common.collect.Sets;
import lombok.Value;
import visitor.Visitor;

import java.util.*;
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
    public Set<String> getGuaranteedChangedVariables() {
        return Sets.newHashSet();
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
    public List<PostfixExpressionInvocationImpl> getInvocations() {
        List<PostfixExpressionInvocationImpl> invocations = new ArrayList<>();
        for (AssignmentExpression assignmentExpression : assignmentExpressions) {
            invocations.addAll(assignmentExpression.getInvocations());
        }
        return invocations;
    }

    @Override
    public void visitNestedExpressions(Visitor<Void, Expression> visitor) {
        for (AssignmentExpression assignmentExpression : assignmentExpressions) {
            visitor.visit(assignmentExpression);
        }
    }
}
