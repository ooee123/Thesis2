package ast;

import visitor.Visitor;

import java.util.Collection;
import java.util.HashSet;
import java.util.Set;

/**
 * Created by ooee on 9/24/16.
 */
public interface BlockItem extends BaseElement {

    boolean isCritical();

    Set<String> getDependantVariables();

    default Set<String> getChangedVariables() {
        Set<String> changedVariables = new HashSet<>();
        changedVariables.addAll(getPotentiallyChangedVariables());
        changedVariables.addAll(getGuaranteedChangedVariables());
        return changedVariables;
    }

    Set<String> getGuaranteedChangedVariables();

    Set<String> getPotentiallyChangedVariables();
/*
    default boolean readsOrWritesMemory() {
        final boolean[] modifiesMemory = new boolean[1];
        modifiesMemory[0] = false;
        Visitor<Expression> visitor = new Visitor<Expression>() {
            @Override
            public void visit(Expression expression) {
                if (expression instanceof UnaryExpressionUnaryOperatorImpl) {
                    if (((UnaryExpressionUnaryOperatorImpl) expression).getUnaryOperator().equals(UnaryExpressionUnaryOperatorImpl.UnaryOperator.DEREFERENCE)) {
                        modifiesMemory[0] = true;
                        return;
                    }
                } else if (expression instanceof PostfixExpressionStructAccessImpl) {
                    if (((PostfixExpressionStructAccessImpl) expression).getAccessOperator().equals(PostfixExpressionStructAccessImpl.AccessOperator.ARROW)) {
                        modifiesMemory[0] = true;
                        return;
                    }
                } else if (expression instanceof PostfixExpressionArrayAccessImpl) {
                    modifiesMemory[0] = true;
                    return;
                }
                expression.visitNestedExpressions(this);
            }
        };
        visitAllExpressions(visitor);
        return modifiesMemory[0];
    }
*/
    default boolean readsMemory() {
        final boolean[] modifiesMemory = new boolean[1];
        modifiesMemory[0] = false;
        Visitor<Expression> visitor = new Visitor<Expression>() {
            @Override
            public void visit(Expression expression) {
                if (expression instanceof UnaryExpressionUnaryOperatorImpl) {
                    if (((UnaryExpressionUnaryOperatorImpl) expression).getUnaryOperator().equals(UnaryExpressionUnaryOperatorImpl.UnaryOperator.DEREFERENCE)) {
                        modifiesMemory[0] = true;
                        return;
                    }
                } else if (expression instanceof PostfixExpressionStructAccessImpl) {
                    if (((PostfixExpressionStructAccessImpl) expression).getAccessOperator().equals(PostfixExpressionStructAccessImpl.AccessOperator.ARROW)) {
                        modifiesMemory[0] = true;
                        return;
                    }
                } else if (expression instanceof PostfixExpressionArrayAccessImpl) {
                    modifiesMemory[0] = true;
                    return;
                }
                expression.visitNestedExpressions(this);
            }
        };
        visitAllExpressions(visitor);
        if (modifiesMemory[0]) {
            return true;
        } else {
            Visitor<Expression> visitorInvocations = new Visitor<Expression>() {
                @Override
                public void visit(Expression expression) {
                    if (expression instanceof PostfixExpressionInvocationImpl) {
                        if (((PostfixExpressionInvocationImpl) expression).readsMemory()) {
                            modifiesMemory[0] = true;
                            return;
                        }
                    }
                    expression.visitNestedExpressions(this);
                }
            };
            visitAllExpressions(visitorInvocations);
            return modifiesMemory[0];
        }
    }

    default boolean writesMemory() {
        final boolean[] modifiesMemory = new boolean[1];
        modifiesMemory[0] = false;
        Visitor<Expression> visitor = new Visitor<Expression>() {
            @Override
            public void visit(Expression expression) {
                if (expression instanceof AssignmentExpressionImpl) {
                    UnaryExpression unaryExpression = ((AssignmentExpressionImpl) expression).getUnaryExpression();
                    if (unaryExpression instanceof UnaryExpressionUnaryOperatorImpl) {
                        if (((UnaryExpressionUnaryOperatorImpl) unaryExpression).getUnaryOperator().equals(UnaryExpressionUnaryOperatorImpl.UnaryOperator.DEREFERENCE)) {
                            modifiesMemory[0] = true;
                            return;
                        }
                    } else if (unaryExpression instanceof PostfixExpressionStructAccessImpl) {
                        if (((PostfixExpressionStructAccessImpl) unaryExpression).getAccessOperator().equals(PostfixExpressionStructAccessImpl.AccessOperator.ARROW)) {
                            modifiesMemory[0] = true;
                            return;
                        }
                    } else if (unaryExpression instanceof PostfixExpressionArrayAccessImpl) {
                        modifiesMemory[0] = true;
                        return;
                    } else if (unaryExpression instanceof PrimaryExpressionParentheses) {
                        visit(((PrimaryExpressionParentheses) unaryExpression).getExpression());
                    }
                }
                expression.visitNestedExpressions(this);
            }
        };
        visitAllExpressions(visitor);
        if (modifiesMemory[0]) {
            return true;
        } else {
            Visitor<Expression> invocationVisitor = new Visitor<Expression>() {
                @Override
                public void visit(Expression expression) {
                    if (expression instanceof PostfixExpressionInvocationImpl) {
                        if (((PostfixExpressionInvocationImpl) expression).writesMemory()) {
                            modifiesMemory[0] = true;
                            return;
                        }
                    }
                    expression.visitNestedExpressions(this);
                }
            };
            visitAllExpressions(invocationVisitor);
            return modifiesMemory[0];
        }
    }

    void visitAllExpressions(Visitor<Expression> visitor);
}
