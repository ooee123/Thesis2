package tst;

import ast.*;
import com.google.common.collect.Lists;
import org.junit.Before;
import org.junit.Test;
import pdg.PDGNode;
import pdg.PDGNodeBreak;
import pdg.PDGNodeComparator;
import pdg.PDGNodeExpressionStatement;

import static org.junit.Assert.*;

/**
 * Created by ooee on 10/27/16.
 */
public class PDGNodeComparatorTest {

    private PDGNodeComparator comparator = new PDGNodeComparator();

    @Before
    public void setup() {

    }

    public PDGNode<? extends BlockItem> getAssignmentPDGNode(String lvalue, ConditionalExpression right) {
        return new PDGNodeExpressionStatement(new ExpressionStatement(new AssignmentExpressionImpl(new PrimaryExpressionIdentifier(lvalue), AssignmentExpressionImpl.AssignmentOperator.ASSIGNMENT, right)));
    }

    @Test
    public void test1() {
        PDGNode<? extends BlockItem> pdgNode1 = getAssignmentPDGNode("a", new PrimaryExpressionConstant(0));
        PDGNode<? extends BlockItem> pdgNode2 = getAssignmentPDGNode("b", new PrimaryExpressionConstant(0));
        assertEquals(0, comparator.compare(pdgNode1, pdgNode2));
    }

    @Test
    public void test2() {
        PDGNode<? extends BlockItem> pdgNode1 = getAssignmentPDGNode("a", new PrimaryExpressionConstant(0));
        PDGNode<? extends BlockItem> pdgNode2 = getAssignmentPDGNode("b", new PrimaryExpressionConstant(1));
        assertEquals(-1, comparator.compare(pdgNode1, pdgNode2));
    }

    @Test
    public void test3() {
        PDGNode<? extends BlockItem> pdgNode1 = getAssignmentPDGNode("a", new PrimaryExpressionConstant(0));
        PDGNode<? extends BlockItem> pdgNode2 = getAssignmentPDGNode("b", new MultiplicativeExpressionImpl(new PrimaryExpressionConstant(1), MultiplicativeExpressionImpl.MultiplicativeOperator.MULTIPLY, new PrimaryExpressionConstant(1)));
        assertEquals(-1, comparator.compare(pdgNode1, pdgNode2));
    }

    @Test
    public void test4() {
        PDGNode<? extends BlockItem> pdgNode1 = getAssignmentPDGNode("a", new AdditiveExpressionImplTree(new PrimaryExpressionConstant(1), AdditiveExpression.AdditiveOperator.PLUS, new PrimaryExpressionConstant(1)));
        PDGNode<? extends BlockItem> pdgNode2 = getAssignmentPDGNode("b", new MultiplicativeExpressionImpl(new PrimaryExpressionConstant(1), MultiplicativeExpressionImpl.MultiplicativeOperator.MULTIPLY, new PrimaryExpressionConstant(1)));
        assertEquals(1, comparator.compare(pdgNode1, pdgNode2));
    }

    @Test
    public void test5() {
        PDGNode<? extends BlockItem> pdgNode1 = getAssignmentPDGNode("a", new AdditiveExpressionImplTree(new PrimaryExpressionConstant(1), AdditiveExpression.AdditiveOperator.PLUS, new PrimaryExpressionConstant(2)));
        PDGNode<? extends BlockItem> pdgNode2 = getAssignmentPDGNode("b", new AdditiveExpressionImplTree(new PrimaryExpressionConstant(2), AdditiveExpression.AdditiveOperator.PLUS, new PrimaryExpressionConstant(1)));
        assertEquals(0, comparator.compare(pdgNode1, pdgNode2));
    }

    @Test
    public void test6() {
        PDGNode<? extends BlockItem> pdgNode1 = getAssignmentPDGNode("a", new AdditiveExpressionImplTree(new PostfixExpressionInvocationImpl(new PrimaryExpressionIdentifier("foo"), Lists.newArrayList()), AdditiveExpression.AdditiveOperator.PLUS, new PrimaryExpressionConstant(2)));
        PDGNode<? extends BlockItem> pdgNode2 = getAssignmentPDGNode("b", new AdditiveExpressionImplTree(new PrimaryExpressionConstant(2), AdditiveExpression.AdditiveOperator.PLUS, new PostfixExpressionInvocationImpl(new PrimaryExpressionIdentifier("foo"), Lists.newArrayList())));
        assertEquals(0, comparator.compare(pdgNode1, pdgNode2));
    }

    @Test
    public void test7() {
        PDGNode<? extends BlockItem> pdgNode1 = getAssignmentPDGNode("a", new MultiplicativeExpressionImpl(new PostfixExpressionInvocationImpl(new PrimaryExpressionIdentifier("foo"), Lists.newArrayList()), MultiplicativeExpressionImpl.MultiplicativeOperator.MULTIPLY, new PrimaryExpressionConstant(2)));
        PDGNode<? extends BlockItem> pdgNode2 = getAssignmentPDGNode("b", new MultiplicativeExpressionImpl(new PrimaryExpressionConstant(2), MultiplicativeExpressionImpl.MultiplicativeOperator.MULTIPLY, new PostfixExpressionInvocationImpl(new PrimaryExpressionIdentifier("foo"), Lists.newArrayList())));
        assertEquals(0, comparator.compare(pdgNode1, pdgNode2));
    }

    @Test
    public void test8() {
        PDGNode<? extends BlockItem> pdgNode1 = getAssignmentPDGNode("a", new AdditiveExpressionImplTree(new PostfixExpressionInvocationImpl(new PrimaryExpressionIdentifier("foo"), Lists.newArrayList()), AdditiveExpression.AdditiveOperator.MINUS, new PrimaryExpressionConstant(2)));
        PDGNode<? extends BlockItem> pdgNode2 = getAssignmentPDGNode("b", new AdditiveExpressionImplTree(new PrimaryExpressionConstant(2), AdditiveExpression.AdditiveOperator.MINUS, new PostfixExpressionInvocationImpl(new PrimaryExpressionIdentifier("foo"), Lists.newArrayList())));
        assertEquals(0, comparator.compare(pdgNode1, pdgNode2));
    }

    @Test
    public void test9() {
        PDGNode<? extends BlockItem> pdgNode1 = getAssignmentPDGNode("a", new AdditiveExpressionImplTree(new PostfixExpressionInvocationImpl(new PrimaryExpressionIdentifier("foo"), Lists.newArrayList()), AdditiveExpression.AdditiveOperator.MINUS, new PrimaryExpressionConstant(2)));
        PDGNode<? extends BlockItem> pdgNode2 = new PDGNodeBreak(new JumpBreakStatement());
        assertEquals(-1, comparator.compare(pdgNode1, pdgNode2));
    }
}