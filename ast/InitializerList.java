package ast;

import lombok.Value;
import visitor.Visitor;

import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * Created by ooee on 9/27/16.
 */
@Value
public class InitializerList implements AssignmentExpression {
    public static final String TOKEN = ",";

    private List<AssignmentExpression> assignmentExpressions;

    @Override
    public String toCode() {
        List<String> codes = assignmentExpressions.stream().map(exp -> exp.toCode()).collect(Collectors.toList());
        return String.join(TOKEN, codes);
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        Set<String> lValues = new HashSet<>();
        for (AssignmentExpression assignmentExpression : assignmentExpressions) {
            lValues.addAll((assignmentExpression).getGuaranteedChangedVariables());
        }
        return lValues;
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
    public Set<String> getDependentVariables() {
        Set<String> rightVariables = new HashSet<>();
        for (AssignmentExpression assignmentExpression : assignmentExpressions) {
            rightVariables.addAll(assignmentExpression.getDependentVariables());
        }
        return rightVariables;
    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        return multiGetInvocations(assignmentExpressions.toArray(new Expression[0]));
    }

    @Override
    public void visitNestedExpressions(Visitor<Expression> visitor) {
        for (AssignmentExpression assignmentExpression : assignmentExpressions) {
            visitor.visit(assignmentExpression);
        }
    }
}
