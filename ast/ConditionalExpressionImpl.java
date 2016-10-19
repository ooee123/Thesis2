package ast;

import lombok.NonNull;
import lombok.Value;

import java.util.HashSet;
import java.util.Set;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class ConditionalExpressionImpl implements ConditionalExpression {
    @NonNull private LogicalOrExpression logicalOrExpression;
    private Expression trueExpression;
    private ConditionalExpression falseExpression;

    @Override
    public String toCode() {
        String trueExpressionCode = "", falseExpressionCode = "";

        if (trueExpression != null) {
            trueExpressionCode = trueExpression.toCode();
        }
        if (falseExpression != null) {
            falseExpressionCode = falseExpression.toCode();
        }
        return logicalOrExpression.toCode() + " ? " + trueExpressionCode + " : " + falseExpressionCode;
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        Set<String> changedVariables = new HashSet<>();
        if (trueExpression != null) {
            changedVariables.addAll(trueExpression.getGuaranteedChangedVariables());
            if (falseExpression != null) {
                changedVariables.retainAll(falseExpression.getGuaranteedChangedVariables());
            }
        }
        if (falseExpression != null) {
            changedVariables.addAll(falseExpression.getGuaranteedChangedVariables());
            if (trueExpression != null) {
                changedVariables.retainAll(trueExpression.getGuaranteedChangedVariables());
            }
        }
        changedVariables.addAll(logicalOrExpression.getGuaranteedChangedVariables());
        return changedVariables;
    }

    @Override
    public Set<String> getPotentiallyChangedVariables() {
        Set<String> potentiallyChangedVariables = new HashSet<>();
        potentiallyChangedVariables.addAll(logicalOrExpression.getGuaranteedChangedVariables());
        if (trueExpression != null) {
            potentiallyChangedVariables.addAll(trueExpression.getGuaranteedChangedVariables());
        }
        if (falseExpression != null) {
            potentiallyChangedVariables.addAll(falseExpression.getGuaranteedChangedVariables());
        }
        potentiallyChangedVariables.removeAll(getGuaranteedChangedVariables());
        return potentiallyChangedVariables;
    }

    @Override
    public Set<String> getDependentVariables() {
        Set<String> dependentVariables = new HashSet<>();
        dependentVariables.addAll(logicalOrExpression.getDependentVariables());
        if (trueExpression != null) {
            dependentVariables.addAll(trueExpression.getDependentVariables());
        }
        if (falseExpression != null) {
            dependentVariables.addAll(falseExpression.getDependentVariables());
        }
        return dependentVariables;
    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        Set<PostfixExpressionInvocationImpl> invocations = new HashSet<>();
        invocations.addAll(logicalOrExpression.getInvocations());
        if (trueExpression != null) {
            invocations.addAll(trueExpression.getInvocations());
        }
        if (falseExpression != null) {
            invocations.addAll(falseExpression.getInvocations());
        }
        return invocations;
    }

    @Override
    public Set<String> getVariables() {
        Set<String> variables = new HashSet<>();
        variables.addAll(logicalOrExpression.getVariables());
        if (trueExpression != null) {
            variables.addAll(trueExpression.getVariables());
        }
        if (falseExpression != null) {
            variables.addAll(falseExpression.getVariables());
        }
        return variables;
    }
}
