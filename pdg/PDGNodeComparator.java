package pdg;

import ast.*;
import lombok.Data;
import lombok.Value;
import pdg.PDGNode;
import visitor.Visitor;

import java.util.*;

/**
 * Created by ooee on 10/25/16.
 */
public class PDGNodeComparator implements Comparator<PDGNode<? extends BlockItem>> {

    @Value
    private static class Score implements Comparable<Score> {
        List<Integer> score;

        public Score() {
            score = new ArrayList<>();
        }

        void addScore(int score) {
            this.score.add(score);
        }

        @Override
        public int compareTo(Score score) {
            if (this.score.size() == score.score.size()) {
                int sum1 = 0;
                for (Integer integer : this.score) {
                    sum1 += integer;
                }
                int sum2 = 0;
                for (Integer integer : score.score) {
                    sum2 += integer;
                }
                if (sum1 != sum2) {
                    return Integer.compare(sum1, sum2);
                }
            }
            for (int i = 0; i < this.score.size() && i < score.score.size(); i++) {
                if (this.score.get(i) != score.score.get(i)) {
                    return Integer.compare(this.score.get(i), score.score.get(i));
                }
            }
            return 0;
        }
    }

    public enum BlockItemStrictOrdering {
        DECLARATION(VariableDeclaration.class),
        EXPRESSION_STATEMENT(ExpressionStatement.class),
        SELECTION_STATEMENT_IF(SelectionStatementIf.class),
        SELECTION_STATEMENT_SWITCH(SelectionStatementSwitch.class),
        ITERATION_STATEMENT_FOR(IterationStatementFor.class),
        ITERATION_STATEMENT_DECLARE_FOR(IterationStatementDeclareFor.class),
        ITERATION_STATEMENT_WHILE(IterationStatementWhile.class),
        ITERATION_STATEMENT_DO_WHILE(IterationStatementDoWhile.class),
        LABELED_IDENTIFIER_STATEMENT(LabeledIdentifierStatement.class),
        LABELED_CASE_STATEMENT(LabeledCaseStatement.class),
        LABELED_DEFAULT_STATEMENT(LabeledDefaultStatement.class),
        COMPOUND_STATEMENT(CompoundStatement.class),
        JUMP_BREAK_STATEMENT(JumpBreakStatement.class),
        JUMP_CONTINUE_STATEMENT(JumpContinueStatement.class),
        JUMP_RETURN_STATEMENT(JumpReturnStatement.class);

        private Class<? extends BlockItem> clazz;
        BlockItemStrictOrdering(Class<? extends BlockItem> clazz) {
            this.clazz = clazz;
        }

        public static int getIndex(BlockItem blockItem) {
            Class<? extends BlockItem> blockItemClass = blockItem.getClass();
            BlockItemStrictOrdering[] values = BlockItemStrictOrdering.values();
            for (int i = 0; i < values.length; i++) {
                if (blockItemClass.equals(values[i].clazz)) {
                    return i;
                }
            }
            throw new IllegalArgumentException("Block Item is not strictly ordered. Received " + blockItemClass);
        }
    }

    @Value
    private static class StatementVisitor implements Visitor<Statement> {
        private Visitor<Expression> visitor;

        StatementVisitor(Visitor<Expression> visitor) {
            this.visitor = visitor;
        }

        @Override
        public void visit(Statement statement) {
            statement.visitAllExpressions(visitor);
        }
    }

    @Value
    private static class ShallowExpressionVisitor implements Visitor<Expression> {
        private Score score;

        ShallowExpressionVisitor() {
            score = new Score();
        }

        public static int distanceToObject(Class clazz) {
            Class[] interfaces = clazz.getInterfaces();
            if (interfaces.length > 0) {
                return 1 + distanceToObject((interfaces[0]));
            }
            return 0;
        }

        @Override
        public void visit(Expression expression) {
            score.addScore(distanceToObject(expression.getClass()));
            expression.visitNestedExpressions(this);
        }
    }

    @Value
    private static class DeepExpressionVisitor implements Visitor<Expression> {
        private Score score;

        DeepExpressionVisitor() {
            score = new Score();
        }

        @Override
        public void visit(Expression expression) {
            GetNumberOperands getNumberOperands = new GetNumberOperands();
            expression.visitNestedExpressions(getNumberOperands);
            for (Integer integer : getNumberOperands.getPoints()) {
                score.addScore(integer);
            }
        }
    }

    @Data
    private static class GetNumberOperands implements Visitor<Expression> {
        private int constantCount;
        private int invocationCount;
        private int identifierCount;
        private List<Integer> points;

        GetNumberOperands() {
            constantCount = 0;
            invocationCount = 0;
            identifierCount = 0;
            points = new ArrayList<>();
        }

        @Override
        public void visit(Expression expression) {
            if (expression instanceof PrimaryExpressionConstant) {
                constantCount += 1;
                Object value = ((PrimaryExpressionConstant) expression).getValue();
                if (value instanceof String) {
                    points.add(3);
                    points.add(((String) value).length());
                }
                if (value instanceof Long) {
                    points.add(0);
                    points.add(((Long) value).intValue());
                }
                if (value instanceof Double) {
                    points.add(1);
                    points.add((int)((Double) value).doubleValue());
                }
                if (value instanceof Character) {
                    points.add(2);
                    points.add((int) ((Character) value).charValue());
                }
            }
            if (expression instanceof PrimaryExpressionIdentifier) {
                points.add(4);
                identifierCount += 1;
            }
            if (expression instanceof PostfixExpressionInvocationImpl) {
                points.add(5);
                invocationCount += 1;
            }
            expression.visitNestedExpressions(this);
        }
    }

    @Override
    public int compare(PDGNode<? extends BlockItem> pdgNode1, PDGNode<? extends BlockItem> pdgNode2) {
        BlockItem b1 = pdgNode1.getBlockItem();
        BlockItem b2 = pdgNode2.getBlockItem();
        if (b1.getClass() != b2.getClass()) {
            return Integer.compare(BlockItemStrictOrdering.getIndex(b1), BlockItemStrictOrdering.getIndex(b2));
        } else {
            if (b1 instanceof VariableDeclaration) {
                Score s1 = new Score(), s2 = new Score();
                for (VariableDeclaration.DeclaredVariable declaredVariable : ((VariableDeclaration) b1).getDeclaredVariables()) {
                    if (declaredVariable.getInitializer() != null) {
                        s1.addScore(ShallowExpressionVisitor.distanceToObject(declaredVariable.getInitializer().getClass()));
                    } else {
                        s1.addScore(0);
                    }
                }
                for (VariableDeclaration.DeclaredVariable declaredVariable : ((VariableDeclaration) b2).getDeclaredVariables()) {
                    if (declaredVariable.getInitializer() != null) {
                        s2.addScore(ShallowExpressionVisitor.distanceToObject(declaredVariable.getInitializer().getClass()));
                    } else {
                        s2.addScore(0);
                    }
                }
                return s1.compareTo(s2);
            } else if (b1 instanceof Statement) {
                ShallowExpressionVisitor v1 = new ShallowExpressionVisitor();
                b1.visitAllExpressions(v1);
                Score score1 = v1.getScore();

                ShallowExpressionVisitor v2 = new ShallowExpressionVisitor();
                b2.visitAllExpressions(v2);
                Score score2 = v2.getScore();

                if (score1.compareTo(score2) == 0) {
                    DeepExpressionVisitor vv1 = new DeepExpressionVisitor();
                    b1.visitAllExpressions(vv1);
                    score1 = vv1.getScore();

                    DeepExpressionVisitor vv2 = new DeepExpressionVisitor();
                    b2.visitAllExpressions(vv2);
                    score2 = vv2.getScore();
                    return score1.compareTo(score2);
                }
                return score2.compareTo(score1);
            }
        }
        throw new IllegalArgumentException();
    }
}
