package ast;

import lombok.Value;

import java.util.HashSet;
import java.util.Set;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class AssignmentExpressionImpl implements AssignmentExpression {

    public enum AssignmentOperator {
        ASSIGNMENT("="),
        MULT_ASSIGNMENT("*="),
        DIV_ASSIGNMENT("/="),
        MOD_ASSIGNMENT("%="),
        ADD_ASSIGNMENT("+="),
        SUB_ASSIGNMENT("-="),
        LEFT_SHIFT_ASSIGNMENT("<<="),
        RIGHT_SHIFT_ASSIGNMENT(">>="),
        BIT_AND_ASSIGNMENT("&="),
        BIT_XOR_ASSIGNMENT("^="),
        BIT_OR_ASSIGNMENT("|=");

        private String token;

        AssignmentOperator(String operator) {
            token = operator;
        }

        public static AssignmentOperator toAssignmentOperator(String token) {
            AssignmentOperator[] assignmentOperators = AssignmentOperator.values();
            for (AssignmentOperator assignmentOperator : assignmentOperators) {
                if (assignmentOperator.getToken().equals(token)) {
                    return assignmentOperator;
                }
            }
            return null;
        }

        public String getToken() {
            return token;
        }
    }

    private UnaryExpression unaryExpression;
    private AssignmentOperator assignmentOperator;
    private AssignmentExpression assignmentExpression;

    @Override
    public String toCode() {
        return unaryExpression.toCode() + assignmentOperator.token + assignmentExpression.toCode();
    }

    @Override
    public Set<String> getChangedVariables() {
        Set<String> lValues = new HashSet<>();
        lValues.addAll(assignmentExpression.getChangedVariables());
        lValues.addAll(unaryExpression.getVariables());
        return lValues;
    }

    @Override
    public Set<String> getDependentVariables() {
        Set<String> dependentVariables = new HashSet<>();
        dependentVariables.addAll(assignmentExpression.getDependentVariables());
        if (unaryExpression instanceof UnaryExpressionUnaryOperatorImpl) {
            UnaryExpressionUnaryOperatorImpl unaryExpression = (UnaryExpressionUnaryOperatorImpl) this.unaryExpression;
            if (unaryExpression.getUnaryOperator().equals(UnaryExpressionUnaryOperatorImpl.UnaryOperator.DEREFERENCE)) {
                // Is a dereference operator
                // We'll do a grand sweeping one. All involved variables on left side are dependent
                dependentVariables.addAll(unaryExpression.getVariables());
            }
        }
        if (unaryExpression instanceof PostfixExpressionArrayAccessImpl) {
            PostfixExpressionArrayAccessImpl unaryExpression = (PostfixExpressionArrayAccessImpl) this.unaryExpression;
            // Array access
            unaryExpression.getDependentVariables();
        }
        if (!assignmentOperator.equals(AssignmentOperator.ASSIGNMENT)) {
            dependentVariables.addAll(unaryExpression.getVariables());
        }
        return dependentVariables;

    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        Set<PostfixExpressionInvocationImpl> invocations = new HashSet<>();
        invocations.addAll(unaryExpression.getInvocations());
        invocations.addAll(assignmentExpression.getInvocations());
        return invocations;
    }

    @Override
    public Set<String> getVariables() {
        Set<String> variables = new HashSet<>();
        variables.addAll(unaryExpression.getVariables());
        variables.addAll(assignmentExpression.getVariables());
        return variables;
    }
}
