package ast.expression.impl;

import ast.expression.BitwiseAndExpression;
import ast.expression.ExclusiveBitwiseOrExpression;
import ast.expression.Expression;
import lombok.Value;
import visitor.Visitor;

import java.util.*;
import java.util.stream.Collectors;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class ExclusiveBitwiseOrExpressionImpl implements ExclusiveBitwiseOrExpression {
    public static final String BITWISE_OR = "^";

    private Collection<BitwiseAndExpression> bitwiseAndExpressions;

    @Override
    public String toCode() {
        List<String> codes = bitwiseAndExpressions.stream().map(exp -> exp.toCode()).collect(Collectors.toList());
        return String.join(" " + BITWISE_OR + " ", codes);
    }

    @Override
    public Set<String> getDependentVariables() {
        return multiGetDependentVariables(bitwiseAndExpressions.toArray(new Expression[0]));
    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        return multiGetInvocations(bitwiseAndExpressions.toArray(new Expression[0]));
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return multiGetGuaranteedChangedVariables(bitwiseAndExpressions.toArray(new Expression[0]));
    }

    @Override
    public Set<String> getVariables() {
        Set<String> variables = new HashSet<>();
        for (BitwiseAndExpression bitwiseAndExpression : bitwiseAndExpressions) {
            variables.addAll(bitwiseAndExpression.getVariables());
        }
        return variables;
    }

    @Override
    public void visitNestedExpressions(Visitor<Expression> visitor) {
        for (BitwiseAndExpression bitwiseAndExpression : bitwiseAndExpressions) {
            visitor.visit(bitwiseAndExpression);
        }
    }
}
