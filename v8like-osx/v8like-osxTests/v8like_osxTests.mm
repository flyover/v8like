//
//  v8like_osxTests.mm
//  v8like-osxTests
//
//  Created by Isaac Burns on 3/15/2014.
//  Copyright (c) 2014 Flyover Games. All rights reserved.
//

#import <XCTest/XCTest.h>

#include <unistd.h>

@interface v8like_osxTests : XCTestCase

@end

@implementation v8like_osxTests

- (void)setUp
{
	[super setUp];
	// Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown
{
	// Put teardown code here. This method is called after the invocation of each test method in the class.
	[super tearDown];
}

- (void)testExample
{
	//XCTFail(@"No implementation for \"%s\"", __PRETTY_FUNCTION__);

	// set working directory to bundle resource path
	NSBundle* bundle = [NSBundle bundleForClass:[self class]];
	//printf("cwd: %s\n", getcwd(NULL, 0));
	chdir([[bundle resourcePath] UTF8String]);
	//printf("cwd: %s\n", getcwd(NULL, 0));

	extern int shell_main(int argc, char* argv[]);
	const char* shell_argv[] = { "shell", "assets/index.js" };
	const int shell_argc = sizeof(shell_argv)/sizeof(shell_argv[0]);
	int shell_err = shell_main(shell_argc, (char**) shell_argv);
	if (shell_err) { XCTFail(@"%d", shell_err); }
}

@end
